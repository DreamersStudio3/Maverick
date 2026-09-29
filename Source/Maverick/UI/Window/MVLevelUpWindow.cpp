#include "UI/Window/MVLevelUpWindow.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVCombatStateComponent.h"
#include "Components/MVStatComponent.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/Progression/MVProgressionDefinition.h"
#include "System/Progression/MVProgressionStatBinding.h"
#include "System/Progression/MVProgressionSubsystem.h"
#include "UI/LevelUp/MVLevelUpAttributeEntryWidget.h"
#include "UI/LevelUp/MVLevelUpStatComparisonWidget.h"
#include "UI/LevelUp/MVLevelUpStatBlockWidget.h"
#include "Components/Button.h"
#include "UI/LevelUp/MVLevelUpSummaryWidget.h"
#include "GameFramework/PlayerController.h"
#include "CommonActionWidget.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"
#include "InputCoreTypes.h"

UMVLevelUpWindow::UMVLevelUpWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UMVLevelUpWindow::NativeOnActivated()
{
	Super::NativeOnActivated();

	PendingRanks.Reset();
	BindRuntimeEvents();

	if (!IsInteractionAllowed())
	{
		DeactivateWidget();
		return;
	}

	RebuildLevelUpRows();
	RefreshLevelUpPreview();
	SetUserFocus(GetOwningPlayer());
}

void UMVLevelUpWindow::NativeOnDeactivated()
{
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(false);
	}

	ClearLevelUpRows();
	UnbindRuntimeEvents();

	PendingRanks.Reset();
	LevelUpPreview = FMVProgressionEvaluation();

	Super::NativeOnDeactivated();
}

void UMVLevelUpWindow::NativeDestruct()
{
	UnregisterLevelUpInputActions();
	
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(
			this,
			&UMVLevelUpWindow::HandleConfirmClicked);
	}

	ClearLevelUpRows();
	UnbindRuntimeEvents();
	PendingRanks.Reset();

	Super::NativeDestruct();
}

bool UMVLevelUpWindow::AdjustPendingRank(const FGameplayTag ProgressionId, const int32 Delta)
{
	TMap<FGameplayTag, int32> CandidateRanks;

	if (!BuildPendingRankCandidate(
		ProgressionId,
		Delta,
		CandidateRanks))
	{
		return false;
	}

	const UMVProgressionSubsystem* Progression = UMVProgressionSubsystem::Get(this);

	if (!Progression
		|| !Progression->EvaluateAllocation(CandidateRanks).IsSuccess())
	{
		return false;
	}

	PendingRanks = MoveTemp(CandidateRanks);
	RefreshLevelUpPreview();

	return true;
}

bool UMVLevelUpWindow::ConfirmLevelUp()
{
	if (!CanConfirmLevelUp())
	{
		return false;
	}

	UMVProgressionSubsystem* Progression = UMVProgressionSubsystem::Get(this);

	if (!Progression)
	{
		return false;
	}

	FMVProgressionEvaluation CommitResult;

	{
		TGuardValue<bool> CommitGuard(bCommitInProgress, true);

		CommitResult = Progression->CommitAllocation(
			PendingRanks,
			LevelUpPreview.SourceRevision);
	}

	if (CommitResult.IsSuccess())
	{
		PendingRanks.Reset();
	}

	RefreshLevelUpPreview();
	BP_LevelUpCommitFinished(CommitResult);

	return CommitResult.IsSuccess();
}

void UMVLevelUpWindow::CancelLevelUp()
{
	if (bCommitInProgress)
	{
		return;
	}

	PendingRanks.Reset();
	DeactivateWidgetWithFade();
}

void UMVLevelUpWindow::RefreshLevelUpPreview()
{
	if (!IsActivated() || bCommitInProgress)
	{
		return;
	}

	UMVProgressionSubsystem* Progression = UMVProgressionSubsystem::Get(this);

	if (Progression)
	{
		LevelUpPreview = Progression->EvaluateAllocation(PendingRanks);
	}
	else
	{
		LevelUpPreview = FMVProgressionEvaluation();
		LevelUpPreview.Result = EMVProgressionEvaluationResult::MissingDefinition;
		LevelUpPreview.Diagnostic = TEXT("Progression subsystem is unavailable.");
	}

	const UMVStatComponent* Stats = PlayerStats.Get();

	PreviewStatRevision = Stats ? Stats->GetStatCalculationRevision() : 0;

	UpdateLevelUpRows();
	UpdateLevelUpSummary();

	BP_LevelUpPreviewChanged(
		LevelUpPreview,
		CanConfirmLevelUp());
}

bool UMVLevelUpWindow::CanConfirmLevelUp() const
{
	if (bCommitInProgress
		|| !bLevelUpRowsReady
		|| !LevelAndCurrencyBlock
		|| !ConfirmButton
		|| !IsInteractionAllowed()
		|| !LevelUpPreview.IsSuccess()
		|| LevelUpPreview.PreviewLevel <= LevelUpPreview.CurrentLevel)
	{
		return false;
	}

	const UMVWorldStateSubsystem* State = WorldState.Get();
	const UMVStatComponent* Stats = PlayerStats.Get();

	if (!State || !Stats
		|| State->GetPlayerProgression().Revision != LevelUpPreview.SourceRevision
		|| Stats->GetStatCalculationRevision() != PreviewStatRevision)
	{
		return false;
	}

	for (const TPair<FGameplayTag, float>& Pair : LevelUpPreview.PreviewStatBonuses)
	{
		float BaseValue = 0.0f;

		if (!FMVProgressionStatBinding::TryGetBaseValue(*Stats, Pair.Key,BaseValue)
			|| !FMath::IsFinite(Pair.Value)
			|| !FMath::IsFinite(BaseValue + Pair.Value))
		{
			return false;
		}
	}

	return true;
}

int32 UMVLevelUpWindow::GetPendingRank(const FGameplayTag ProgressionId) const
{
	return PendingRanks.FindRef(ProgressionId);
}

TArray<FMVProgressionEntryDefinition> UMVLevelUpWindow::GetProgressionEntries() const
{
	const UMVProgressionSubsystem* Progression = UMVProgressionSubsystem::Get(this);
	const UMVProgressionDefinition* Definition = Progression ? Progression->GetDefinition() : nullptr;

	if (!Definition)
	{
		return {};
	}

	TArray<FMVProgressionEntryDefinition> Entries = Definition->Entries;

	Entries.Sort(
		[](const FMVProgressionEntryDefinition& Left,
			const FMVProgressionEntryDefinition& Right)
		{
			return Left.SortOrder < Right.SortOrder;
		});

	return Entries;
}

void UMVLevelUpWindow::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandleConfirmClicked);

		ConfirmButton->SetIsEnabled(CanConfirmLevelUp());
	}
	
	RegisterLevelUpInputActions();
}

bool UMVLevelUpWindow::IsInteractionAllowed() const
{
	const AMVPlayerCharacter* Player = Cast<AMVPlayerCharacter>(GetOwningPlayerPawn());

	return IsActivated()
		&& !IsFadingOut()
		&& Player
		&& PlayerStats.Get() == Player->StatComponent
		&& PlayerCombatState.Get() == Player->CombatStateComponent
		&& FieldTransition.IsValid()
		&& CanOpenForPlayer(GetOwningPlayer());
}

void UMVLevelUpWindow::BindRuntimeEvents()
{
	UnbindRuntimeEvents();

	AMVPlayerCharacter* Player = Cast<AMVPlayerCharacter>(GetOwningPlayerPawn());

	if (!Player)
	{
		return;
	}

	PlayerStats = Player->StatComponent;
	PlayerCombatState = Player->CombatStateComponent;
	WorldState = UMVWorldStateSubsystem::Get(this);
	FieldTransition = UMVFieldTransitionSubsystem::Get(this);

	if (UMVWorldStateSubsystem* State = WorldState.Get())
	{
		State->OnPlayerProgressionChanged.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandleProgressionChanged);
	}

	if (UMVStatComponent* Stats = PlayerStats.Get())
	{
		Stats->OnBaseStatsReady.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandleBaseStatsReady);

		Stats->OnDead.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandlePlayerDead);
	}

	if (UMVCombatStateComponent* Combat = PlayerCombatState.Get())
	{
		Combat->OnCombatStateChanged.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandleCombatStateChanged);
	}

	if (UMVFieldTransitionSubsystem* Transition = FieldTransition.Get())
	{
		Transition->OnFieldTransitionPhaseChanged.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandleTransitionPhaseChanged);
	}
}

void UMVLevelUpWindow::UnbindRuntimeEvents()
{
	if (UMVWorldStateSubsystem* State = WorldState.Get())
	{
		State->OnPlayerProgressionChanged.RemoveDynamic(
			this,
			&UMVLevelUpWindow::HandleProgressionChanged);
	}

	if (UMVStatComponent* Stats = PlayerStats.Get())
	{
		Stats->OnBaseStatsReady.RemoveDynamic(
			this,
			&UMVLevelUpWindow::HandleBaseStatsReady);

		Stats->OnDead.RemoveDynamic(
			this,
			&UMVLevelUpWindow::HandlePlayerDead);
	}

	if (UMVCombatStateComponent* Combat = PlayerCombatState.Get())
	{
		Combat->OnCombatStateChanged.RemoveDynamic(
			this,
			&UMVLevelUpWindow::HandleCombatStateChanged);
	}

	if (UMVFieldTransitionSubsystem* Transition = FieldTransition.Get())
	{
		Transition->OnFieldTransitionPhaseChanged.RemoveDynamic(
			this,
			&UMVLevelUpWindow::HandleTransitionPhaseChanged);
	}

	WorldState.Reset();
	PlayerStats.Reset();
	PlayerCombatState.Reset();
	FieldTransition.Reset();
}

void UMVLevelUpWindow::HandleProgressionChanged(const FMVPlayerProgressionSaveData& PlayerProgression)
{
	(void)PlayerProgression;

	if (bCommitInProgress)
	{
		return;
	}

	PendingRanks.Reset();
	RefreshLevelUpPreview();
}

void UMVLevelUpWindow::HandleBaseStatsReady(const int32 Revision)
{
	(void)Revision;

	PendingRanks.Reset();
	RefreshLevelUpPreview();
}

void UMVLevelUpWindow::HandlePlayerDead()
{
	DeactivateWidget();
}

void UMVLevelUpWindow::HandleCombatStateChanged(const bool bInCombat)
{
	if (bInCombat)
	{
		DeactivateWidget();
	}
	else
	{
		RefreshLevelUpPreview();
	}
}

void UMVLevelUpWindow::HandleTransitionPhaseChanged(const EMVFieldTransitionPhase NewPhase)
{
	if (NewPhase != EMVFieldTransitionPhase::Idle)
	{
		DeactivateWidget();
	}
}

bool UMVLevelUpWindow::BuildPendingRankCandidate(
	const FGameplayTag ProgressionId,
	const int32 Delta,
	TMap<FGameplayTag, int32>& OutCandidateRanks) const
{
	OutCandidateRanks.Reset();

	if (bCommitInProgress
		|| !bLevelUpRowsReady
		|| !IsInteractionAllowed()
		|| (Delta != 1 && Delta != -1))
	{
		return false;
	}

	const UMVProgressionSubsystem* Progression = UMVProgressionSubsystem::Get(this);
	const UMVProgressionDefinition* Definition = Progression ? Progression->GetDefinition() : nullptr;

	if (!Definition || !Definition->FindEntry(ProgressionId))
	{
		return false;
	}

	const int64 NewPendingRank = static_cast<int64>(PendingRanks.FindRef(ProgressionId)) + Delta;

	if (NewPendingRank < 0 || NewPendingRank > MAX_int32)
	{
		return false;
	}

	OutCandidateRanks = PendingRanks;

	if (NewPendingRank == 0)
	{
		OutCandidateRanks.Remove(ProgressionId);
	}
	else
	{
		OutCandidateRanks.Add(ProgressionId, static_cast<int32>(NewPendingRank));
	}

	return true;
}

bool UMVLevelUpWindow::CanAdjustPendingRank(const FGameplayTag ProgressionId, const int32 Delta) const
{
	TMap<FGameplayTag, int32> CandidateRanks;

	if (!BuildPendingRankCandidate(ProgressionId, Delta, CandidateRanks))
	{
		return false;
	}

	const UMVProgressionSubsystem* Progression = UMVProgressionSubsystem::Get(this);

	return Progression && Progression->EvaluateAllocation(CandidateRanks).IsSuccess();
}

bool UMVLevelUpWindow::RebuildLevelUpRows()
{
	ClearLevelUpRows();

	if (!AttributeBlock
		|| !ComparisonBlock
		|| !AttributeEntryClass
		|| !ComparisonEntryClass)
	{
		return false;
	}

	const TArray<FMVProgressionEntryDefinition> Entries = GetProgressionEntries();

	if (Entries.IsEmpty())
	{
		return false;
	}

	for (const FMVProgressionEntryDefinition& Entry : Entries)
	{
		if (!Entry.ProgressionId.IsValid()
			|| AttributeRows.Contains(Entry.ProgressionId))
		{
			ClearLevelUpRows();
			return false;
		}

		UMVLevelUpAttributeEntryWidget* AttributeRow = AttributeBlock->AddAttributeEntry(AttributeEntryClass);

		if (!AttributeRow)
		{
			ClearLevelUpRows();
			return false;
		}

		AttributeRows.Add(Entry.ProgressionId, AttributeRow);
		OrderedAttributeIds.Add(Entry.ProgressionId);

		AttributeRow->OnSelectionRequested.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandleAttributeSelectionRequested);
		
		AttributeRow->OnRankAdjustmentRequested.AddUniqueDynamic(
			this,
			&UMVLevelUpWindow::HandleRankAdjustmentRequested);

		for (const FMVProgressionStatEffect& Effect : Entry.Effects)
		{
			if (ComparisonRows.Contains(Effect.StatId))
			{
				continue;
			}

			FText DisplayName;
			bool bLowerIsBetter = false;

			if (!FMVProgressionStatBinding::TryGetDisplayInfo(
				Effect.StatId,
				DisplayName,
				bLowerIsBetter))
			{
				ClearLevelUpRows();
				return false;
			}

			UMVLevelUpStatComparisonWidget* ComparisonRow = ComparisonBlock->AddComparisonEntry(ComparisonEntryClass);

			if (!ComparisonRow)
			{
				ClearLevelUpRows();
				return false;
			}

			ComparisonRows.Add(Effect.StatId, ComparisonRow);
		}
	}

	bLevelUpRowsReady = !ComparisonRows.IsEmpty();

	if (!bLevelUpRowsReady)
	{
		ClearLevelUpRows();
	}
	else
	{
		SelectedAttributeIndex = 0;
		UpdateAttributeSelection();
	}

	return bLevelUpRowsReady;
}

void UMVLevelUpWindow::ClearLevelUpRows()
{
	bLevelUpRowsReady = false;

	for (const auto& Pair : AttributeRows)
	{
		if (UMVLevelUpAttributeEntryWidget* Row = Pair.Value.Get())
		{
			Row->OnRankAdjustmentRequested.RemoveDynamic(
				this,
				&UMVLevelUpWindow::HandleRankAdjustmentRequested);
			
			Row->OnSelectionRequested.RemoveDynamic(
				this,
				&UMVLevelUpWindow::HandleAttributeSelectionRequested);
		}
	}

	AttributeRows.Reset();
	ComparisonRows.Reset();
	OrderedAttributeIds.Reset();
	SelectedAttributeIndex = INDEX_NONE;
	
	if (AttributeBlock)
	{
		AttributeBlock->ClearEntries();
	}

	if (ComparisonBlock)
	{
		ComparisonBlock->ClearEntries();
	}
}

void UMVLevelUpWindow::UpdateLevelUpRows()
{
	const UMVWorldStateSubsystem* State = WorldState.Get();
	const UMVStatComponent* Stats = PlayerStats.Get();

	const bool bCanDisplay =
		bLevelUpRowsReady
		&& LevelUpPreview.IsSuccess()
		&& State
		&& Stats;

	if (AttributeBlock)
	{
		AttributeBlock->SetVisibility(
			bCanDisplay ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

		AttributeBlock->SetIsEnabled(bCanDisplay && IsInteractionAllowed());
	}

	if (ComparisonBlock)
	{
		ComparisonBlock->SetVisibility(
			bCanDisplay ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (!bCanDisplay)
	{
		return;
	}

	const FMVPlayerProgressionSaveData& SaveData = State->GetPlayerProgression();

	for (const FMVProgressionEntryDefinition& Entry : GetProgressionEntries())
	{
		const auto* FoundRow = AttributeRows.Find(Entry.ProgressionId);

		if (!FoundRow || !FoundRow->Get())
		{
			continue;
		}

		FoundRow->Get()->SetEntry(
			Entry,
			SaveData.InvestedRanks.FindRef(Entry.ProgressionId),
			LevelUpPreview.PreviewRanks.FindRef(Entry.ProgressionId),
			CanAdjustPendingRank(Entry.ProgressionId, -1),
			CanAdjustPendingRank(Entry.ProgressionId, 1));
	}

	for (const auto& Pair : ComparisonRows)
	{
		UMVLevelUpStatComparisonWidget* Row = Pair.Value.Get();

		if (!Row)
		{
			continue;
		}

		FText DisplayName;
		bool bLowerIsBetter = false;
		float CurrentValue = 0.0f;

		if (!FMVProgressionStatBinding::TryGetDisplayInfo(Pair.Key, DisplayName, bLowerIsBetter)
			|| !FMVProgressionStatBinding::TryGetEffectiveValue(*Stats, Pair.Key, CurrentValue))
		{
			Row->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		const float BonusChange =
			LevelUpPreview.PreviewStatBonuses.FindRef(Pair.Key) - LevelUpPreview.CurrentStatBonuses.FindRef(Pair.Key);

		const bool bDisplayed = Row->SetComparison(
			Pair.Key,
			DisplayName,
			CurrentValue,
			CurrentValue + BonusChange,
			bLowerIsBetter);

		Row->SetVisibility(bDisplayed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UMVLevelUpWindow::UpdateLevelUpSummary()
{
	if (LevelAndCurrencyBlock)
	{
		LevelAndCurrencyBlock->SetSummary(LevelUpPreview);
	}

	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(CanConfirmLevelUp());
	}
}

void UMVLevelUpWindow::HandleRankAdjustmentRequested(
	const FGameplayTag ProgressionId,
	const int32 Delta)
{
	HandleAttributeSelectionRequested(ProgressionId);
	AdjustPendingRank(ProgressionId, Delta);
}

void UMVLevelUpWindow::HandleConfirmClicked()
{
	ConfirmLevelUp();
}

bool UMVLevelUpWindow::CanOpenForPlayer(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController)
		|| !PlayerController->IsLocalController())
	{
		return false;
	}

	const UWorld* World = PlayerController->GetWorld();

	if (!World || !World->IsGameWorld())
	{
		return false;
	}

	const AMVPlayerCharacter* Player = Cast<AMVPlayerCharacter>(PlayerController->GetPawn());

	if (!IsValid(Player))
	{
		return false;
	}

	const UMVStatComponent* Stats = Player->StatComponent;
	const UMVCombatStateComponent* Combat = Player->CombatStateComponent;
	const UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(Player);

	return Stats
		&& Stats->IsBaseStatsReady()
		&& !Stats->IsDead()
		&& Combat
		&& Combat->IsOutOfCombat()
		&& Transition
		&& !Transition->IsTransitionRunning();
}

void UMVLevelUpWindow::RegisterLevelUpInputActions()
{
	UnregisterLevelUpInputActions();

	if (IsDesignTime())
	{
		return;
	}

	const UCommonInputSettings& InputSettings = ICommonInputModule::GetSettings();
	const FDataTableRowHandle ConfirmAction = InputSettings.GetDefaultClickAction();

	if (!ConfirmAction.IsNull())
	{
		const EInputEvent InputEvents[] =
		{
			IE_Pressed,
			IE_Repeat,
			IE_Released
		};

		for (const EInputEvent InputEvent : InputEvents)
		{
			FBindUIActionArgs BindArgs(
				ConfirmAction,
				false,
				FSimpleDelegate::CreateUObject(
					this,
					&UMVLevelUpWindow::HandleConfirmInput,
					InputEvent));

			BindArgs.InputMode = ECommonInputMode::Menu;
			BindArgs.KeyEvent = InputEvent;
			BindArgs.bConsumeInput = true;

			const FUIActionBindingHandle Handle = RegisterUIActionBinding(BindArgs);

			if (Handle.IsValid())
			{
				ConfirmInputBindings.Add(Handle);
			}
		}
	}

	if (ConfirmInputHint)
	{
		ConfirmInputHint->SetInputAction(ConfirmAction);
	}

	if (CancelInputHint)
	{
		CancelInputHint->SetInputAction(InputSettings.GetDefaultBackAction());
	}
}

void UMVLevelUpWindow::UnregisterLevelUpInputActions()
{
	for (FUIActionBindingHandle& Handle : ConfirmInputBindings)
	{
		RemoveActionBinding(Handle);
		Handle.Unregister();
	}

	ConfirmInputBindings.Reset();
}

void UMVLevelUpWindow::HandleConfirmInput(const EInputEvent InputEvent)
{
	if (InputEvent == IE_Pressed && IsInteractionAllowed())
	{
		ConfirmLevelUp();
	}
}

UWidget* UMVLevelUpWindow::NativeGetDesiredFocusTarget() const
{
	return const_cast<UMVLevelUpWindow*>(this);
}

bool UMVLevelUpWindow::NativeOnHandleBackAction()
{
	CancelLevelUp();
	return true;
}

FReply UMVLevelUpWindow::NativeOnPreviewKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (!IsActivated())
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}

	const FKey Key = InKeyEvent.GetKey();
	EUINavigation Direction = EUINavigation::Invalid;

	if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up)
	{
		Direction = EUINavigation::Up;
	}
	else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down)
	{
		Direction = EUINavigation::Down;
	}
	else if (Key == EKeys::Left || Key == EKeys::Gamepad_DPad_Left)
	{
		Direction = EUINavigation::Left;
	}
	else if (Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Right)
	{
		Direction = EUINavigation::Right;
	}

	if (HandleLevelUpNavigation(Direction))
	{
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FNavigationReply UMVLevelUpWindow::NativeOnNavigation(
	const FGeometry& InGeometry,
	const FNavigationEvent& InNavigationEvent,
	const FNavigationReply& InDefaultReply)
{
	if (IsActivated()
		&& HandleLevelUpNavigation(InNavigationEvent.GetNavigationType()))
	{
		return FNavigationReply::Stop();
	}

	return Super::NativeOnNavigation(
		InGeometry,
		InNavigationEvent,
		InDefaultReply);
}

bool UMVLevelUpWindow::HandleLevelUpNavigation(const EUINavigation Direction)
{
	const bool bMoveUp =
		Direction == EUINavigation::Up
		|| Direction == EUINavigation::Previous;

	const bool bMoveDown =
		Direction == EUINavigation::Down
		|| Direction == EUINavigation::Next;

	const bool bDecrease = Direction == EUINavigation::Left;
	const bool bIncrease = Direction == EUINavigation::Right;

	if (!bMoveUp && !bMoveDown && !bDecrease && !bIncrease)
	{
		return false;
	}

	if (!IsInteractionAllowed()
		|| bCommitInProgress
		|| !bLevelUpRowsReady
		|| !LevelUpPreview.IsSuccess()
		|| !OrderedAttributeIds.IsValidIndex(SelectedAttributeIndex))
	{
		return true;
	}

	if (bMoveUp || bMoveDown)
	{
		const int32 Delta = bMoveDown ? 1 : -1;

		SelectedAttributeIndex = FMath::Clamp(
			SelectedAttributeIndex + Delta,
			0,
			OrderedAttributeIds.Num() - 1);

		UpdateAttributeSelection();
	}
	else
	{
		AdjustPendingRank(
			OrderedAttributeIds[SelectedAttributeIndex],
			bIncrease ? 1 : -1);
	}

	return true;
}

void UMVLevelUpWindow::HandleAttributeSelectionRequested(const FGameplayTag ProgressionId)
{
	if (!IsInteractionAllowed() || bCommitInProgress || !bLevelUpRowsReady)
	{
		return;
	}

	const int32 FoundIndex = OrderedAttributeIds.IndexOfByKey(ProgressionId);

	if (FoundIndex == INDEX_NONE)
	{
		return;
	}

	SelectedAttributeIndex = FoundIndex;
	UpdateAttributeSelection();
}

void UMVLevelUpWindow::UpdateAttributeSelection()
{
	const FGameplayTag SelectedId =
		OrderedAttributeIds.IsValidIndex(SelectedAttributeIndex)
		? OrderedAttributeIds[SelectedAttributeIndex]
		: FGameplayTag();

	for (const auto& Pair : AttributeRows)
	{
		if (UMVLevelUpAttributeEntryWidget* Row = Pair.Value.Get())
		{
			Row->SetSelected(Pair.Key == SelectedId);
		}
	}
}