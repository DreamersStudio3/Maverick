#include "System/MVFieldTransitionSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Character/PC/Consumable/MVPlayerConsumableComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVDeathComponent.h"
#include "Components/MVStatComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "System/MVDeathRespawnFlow.h"
#include "System/MVFieldTransitionResettableInterface.h"
#include "System/MVFieldTransitionSettings.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/MVWorldStateTypes.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVLoadingWindow.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVFieldTransitionSubsystem, Log, All);

namespace
{
FName MVFieldTransitionActionTableNameFromDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}
}

void UMVFieldTransitionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UMVWorldStateSubsystem::StaticClass());

	Super::Initialize(Collection);

	DeathRespawnFlow = NewObject<UMVDeathRespawnFlow>(this);
	if (DeathRespawnFlow)
	{
		DeathRespawnFlow->Initialize(this);
	}
}

void UMVFieldTransitionSubsystem::Deinitialize()
{
	if (DeathRespawnFlow)
	{
		DeathRespawnFlow->Shutdown();
		DeathRespawnFlow = nullptr;
	}
	ResetTransitionState();

	Super::Deinitialize();
}

UMVFieldTransitionSubsystem* UMVFieldTransitionSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVFieldTransitionSubsystem>() : nullptr;
}

bool UMVFieldTransitionSubsystem::StartTransition(const FMVFieldTransitionRequest& Request)
{
	if (TransitionPhase != EMVFieldTransitionPhase::Idle)
	{
		return false;
	}

	ActiveTransitionRequest = Request;
	if (ActiveTransitionRequest.PreTransitionActionRow.DataTable
		&& !ActiveTransitionRequest.PreTransitionActionRow.RowName.IsNone())
	{
		ClearUIBeforeLoadingIfNeeded();
		SetTransitionPhase(EMVFieldTransitionPhase::PreTransition);
		if (TryStartPreTransitionAction())
		{
			return true;
		}
	}

	BeginLoadingReset();
	return true;
}

void UMVFieldTransitionSubsystem::BeginLoadingReset()
{
	SetTransitionPhase(EMVFieldTransitionPhase::LoadingReset);
	ClearUIBeforeLoadingIfNeeded();

	UMVUISubsystem* UISubsystem = GetUISubsystem();
	ActiveLoadingWindow = UISubsystem
		? Cast<UMVLoadingWindow>(UISubsystem->ShowLoadingWindow())
		: nullptr;

	UpdateTransitionProgress(0.15f, NSLOCTEXT("MaverickFieldTransition", "LoadingPrepare", "필드 전환 준비 중"));
	const int32 ResetActorCount = ActiveTransitionRequest.bResetFieldActors
		? ResetWorldActorsForTransition(ActiveTransitionRequest)
		: 0;
	UpdateTransitionProgress(
		0.45f,
		FText::Format(
			NSLOCTEXT("MaverickFieldTransition", "FieldReset", "필드 상태 초기화 대상 {0}개 적용"),
			FText::AsNumber(ResetActorCount)));
	UpdateTransitionProgress(0.75f, NSLOCTEXT("MaverickFieldTransition", "WorldState", "저장 상태 적용 준비"));
	StartAutomaticLoadingCompletion();
}

bool UMVFieldTransitionSubsystem::StartDeathRespawnTransition(AActor* DeadActor)
{
	FMVFieldTransitionRequest Request;
	Request.Reason = EMVFieldTransitionReason::DeathRespawn;
	Request.SourceActor = DeadActor;
	Request.bUseLastCheckpoint = true;
	Request.bResetFieldActors = true;
	Request.bResetDeathPresentation = true;
	Request.bRestorePlayerStats = true;
	Request.bClearUIBeforeLoading = false;
	Request.bResetUIToDefaultOnComplete = true;
	Request.bRestoreInputOnComplete = true;
	if (const UMVFieldTransitionSettings* Settings = GetDefault<UMVFieldTransitionSettings>())
	{
		Request.PostTransitionActionRow = Settings->DeathRespawnPostTransitionActionRow;
		Request.PostTransitionStartSection = Settings->DeathRespawnPostTransitionStartSection;
	}
	return StartTransition(Request);
}

bool UMVFieldTransitionSubsystem::StartCheckpointTravelToTransform(
	const FName CheckpointId,
	const FName FieldId,
	const FTransform& TargetTransform,
	const bool bSaveAsLastCheckpoint)
{
	if (bSaveAsLastCheckpoint)
	{
		UMVWorldStateSubsystem* WorldState = GetWorldState();
		if (!WorldState
			|| !WorldState->SetLastCheckpoint(CheckpointId, FieldId, TargetTransform, NAME_None)
			|| !WorldState->SaveToDefaultSlot())
		{
			return false;
		}
	}

	FMVFieldTransitionRequest Request;
	Request.Reason = EMVFieldTransitionReason::CheckpointTravel;
	Request.TargetCheckpointId = CheckpointId;
	Request.TargetFieldId = FieldId;
	Request.TargetTransform = TargetTransform;
	Request.bHasTargetTransform = true;
	Request.bUseLastCheckpoint = false;
	Request.bResetFieldActors = true;
	Request.bResetDeathPresentation = false;
	Request.bRestorePlayerStats = false;
	Request.bClearUIBeforeLoading = true;
	Request.bResetUIToDefaultOnComplete = true;
	Request.bRestoreInputOnComplete = true;
	if (const UMVFieldTransitionSettings* Settings = GetDefault<UMVFieldTransitionSettings>())
	{
		Request.PreTransitionActionRow = Settings->CheckpointTravelPreTransitionActionRow;
		Request.PreTransitionStartSection = Settings->CheckpointTravelPreTransitionStartSection;
		Request.PostTransitionActionRow = Settings->CheckpointTravelPostTransitionActionRow;
		Request.PostTransitionStartSection = Settings->CheckpointTravelPostTransitionStartSection;
	}
	return StartTransition(Request);
}

void UMVFieldTransitionSubsystem::CompleteTransition()
{
	if (TransitionPhase == EMVFieldTransitionPhase::Idle)
	{
		return;
	}
	if (TransitionPhase == EMVFieldTransitionPhase::PreTransition)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutomaticLoadingTimerHandle);
		World->GetTimerManager().ClearTimer(ScreenTransitionCompletionTimerHandle);
	}

	SetTransitionPhase(EMVFieldTransitionPhase::Relocating);
	UpdateTransitionProgress(1.0f, NSLOCTEXT("MaverickFieldTransition", "Complete", "전환 위치 적용"));
	ApplyTransitionDestination(ActiveTransitionRequest);
	if (AMVCharacterBase* Character = ResolvePlayerCharacter(GetWorld()))
	{
		TryPreparePostTransitionAction(*Character, ActiveTransitionRequest);
	}

	float ScreenTransitionDelaySeconds = 0.0f;
	if (ActiveLoadingWindow)
	{
		ScreenTransitionDelaySeconds = ActiveLoadingWindow->GetFadeOutSeconds();
		ActiveLoadingWindow->DeactivateWidgetWithFade();
		ActiveLoadingWindow = nullptr;
	}

	SetTransitionPhase(EMVFieldTransitionPhase::Completing);

	ScreenTransitionDelaySeconds = FMath::Max(0.0f, ScreenTransitionDelaySeconds);
	if (ScreenTransitionDelaySeconds > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				ScreenTransitionCompletionTimerHandle,
				this,
				&UMVFieldTransitionSubsystem::FinishTransitionAfterScreenChange,
				ScreenTransitionDelaySeconds,
				false);
			return;
		}
	}

	FinishTransitionAfterScreenChange();
}

void UMVFieldTransitionSubsystem::FinishTransitionAfterScreenChange()
{
	if (TransitionPhase == EMVFieldTransitionPhase::Idle)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScreenTransitionCompletionTimerHandle);
	}

	if (ActiveTransitionRequest.bResetUIToDefaultOnComplete)
	{
		ResetUIToDefaultAfterTransition();
	}
	if (ActiveTransitionRequest.bRestoreInputOnComplete)
	{
		RestorePlayerInputAfterTransition();
	}
	ResumePreparedPostTransitionAction();

	ResetTransitionState();
	SetTransitionPhase(EMVFieldTransitionPhase::Idle);
}

bool UMVFieldTransitionSubsystem::SetLastCheckpointFromActor(
	AActor* CheckpointActor,
	const FName CheckpointId,
	const FName FieldId,
	const bool bSaveImmediately)
{
	UMVWorldStateSubsystem* WorldState = GetWorldState();
	if (!CheckpointActor || !WorldState)
	{
		return false;
	}

	if (!WorldState->SetLastCheckpoint(CheckpointId, FieldId, CheckpointActor->GetActorTransform(), NAME_None))
	{
		return false;
	}

	return !bSaveImmediately || WorldState->SaveToDefaultSlot();
}

void UMVFieldTransitionSubsystem::HandlePreTransitionActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	const bool /*bInterrupted*/)
{
	if (TransitionPhase != EMVFieldTransitionPhase::PreTransition)
	{
		return;
	}

	const FName ExpectedTableName =
		MVFieldTransitionActionTableNameFromDataTable(ActiveTransitionRequest.PreTransitionActionRow.DataTable);
	const FName ExpectedRowName = ActiveTransitionRequest.PreTransitionActionRow.RowName;
	if ((!ExpectedTableName.IsNone() && ActionTableName != ExpectedTableName)
		|| (!ExpectedRowName.IsNone() && ActionRowName != ExpectedRowName))
	{
		return;
	}

	UnbindPreTransitionAction();
	BeginLoadingReset();
}

void UMVFieldTransitionSubsystem::SetTransitionPhase(const EMVFieldTransitionPhase NewPhase)
{
	if (TransitionPhase == NewPhase)
	{
		return;
	}

	TransitionPhase = NewPhase;
	OnFieldTransitionPhaseChanged.Broadcast(TransitionPhase);
}

void UMVFieldTransitionSubsystem::ClearUIBeforeLoadingIfNeeded()
{
	if (bClearedUIBeforeLoading || !ActiveTransitionRequest.bClearUIBeforeLoading)
	{
		return;
	}

	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->ClearAllUI(true);
		bClearedUIBeforeLoading = true;
	}
}

bool UMVFieldTransitionSubsystem::TryStartPreTransitionAction()
{
	AMVCharacterBase* Character = ResolvePlayerCharacter(GetWorld());
	UMVActionComponent* ActionComponent = Character ? Character->ActionComponent : nullptr;
	if (!ActionComponent)
	{
		UE_LOG(
			LogMVFieldTransitionSubsystem,
			Warning,
			TEXT("Pre-transition action requested, but player action component was not resolved."));
		return false;
	}

	PreTransitionActionComponent = ActionComponent;
	ActionComponent->OnActionEnded.RemoveDynamic(this, &UMVFieldTransitionSubsystem::HandlePreTransitionActionEnded);
	ActionComponent->OnActionEnded.AddUniqueDynamic(this, &UMVFieldTransitionSubsystem::HandlePreTransitionActionEnded);

	const bool bStarted = ActionComponent->TryStartActionFromRowHandle(
		ActiveTransitionRequest.PreTransitionActionRow,
		ActiveTransitionRequest.PreTransitionStartSection);
	if (!bStarted)
	{
		UnbindPreTransitionAction();
		UE_LOG(
			LogMVFieldTransitionSubsystem,
			Warning,
			TEXT("Pre-transition action failed to start. Table=%s, RowName=%s."),
			*GetNameSafe(ActiveTransitionRequest.PreTransitionActionRow.DataTable),
			*ActiveTransitionRequest.PreTransitionActionRow.RowName.ToString());
	}

	return bStarted;
}

void UMVFieldTransitionSubsystem::UnbindPreTransitionAction()
{
	if (PreTransitionActionComponent)
	{
		PreTransitionActionComponent->OnActionEnded.RemoveDynamic(
			this,
			&UMVFieldTransitionSubsystem::HandlePreTransitionActionEnded);
		PreTransitionActionComponent = nullptr;
	}
}

void UMVFieldTransitionSubsystem::UpdateTransitionProgress(const float Progress, FText StepText)
{
	const float ClampedProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	if (ActiveLoadingWindow)
	{
		ActiveLoadingWindow->SetLoadingProgress(ClampedProgress, StepText);
	}

	OnFieldTransitionProgressChanged.Broadcast(ClampedProgress, StepText);
}

void UMVFieldTransitionSubsystem::StartAutomaticLoadingCompletion()
{
	if (!bCompleteLoadingAutomatically)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		CompleteTransition();
		return;
	}

	const float DelaySeconds = FMath::Max(0.0f, MinimumLoadingSeconds);
	if (DelaySeconds <= 0.0f)
	{
		CompleteTransition();
		return;
	}

	World->GetTimerManager().SetTimer(
		AutomaticLoadingTimerHandle,
		this,
		&UMVFieldTransitionSubsystem::CompleteTransition,
		DelaySeconds,
		false);
}

void UMVFieldTransitionSubsystem::ResetTransitionState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutomaticLoadingTimerHandle);
		World->GetTimerManager().ClearTimer(ScreenTransitionCompletionTimerHandle);
	}

	UnbindPreTransitionAction();
	PostTransitionActionComponent = nullptr;
	ActiveTransitionRequest = FMVFieldTransitionRequest();
	ActiveLoadingWindow = nullptr;
	bClearedUIBeforeLoading = false;
}

int32 UMVFieldTransitionSubsystem::ResetWorldActorsForTransition(const FMVFieldTransitionRequest& Request)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	UMVWorldStateSubsystem* WorldState = GetWorldState();
	const FName TargetFieldId = ResolveTransitionResetFieldId(Request);
	AMVCharacterBase* PlayerCharacter = ResolvePlayerCharacter(World);
	int32 ResetActorCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor) || Actor == PlayerCharacter || Actor == Request.SourceActor)
		{
			continue;
		}

		if (!Actor->GetClass()->ImplementsInterface(UMVFieldTransitionResettableInterface::StaticClass()))
		{
			continue;
		}

		const FName ActorFieldId = IMVFieldTransitionResettableInterface::Execute_GetFieldTransitionResetFieldId(Actor);
		if (!TargetFieldId.IsNone() && !ActorFieldId.IsNone() && ActorFieldId != TargetFieldId)
		{
			continue;
		}

		const EMVFieldTransitionResetPolicy ResetPolicy =
			IMVFieldTransitionResettableInterface::Execute_GetFieldTransitionResetPolicy(Actor);
		const FName ObjectId = IMVFieldTransitionResettableInterface::Execute_GetFieldTransitionResetObjectId(Actor);
		const FName ContextFieldId = ActorFieldId.IsNone() ? TargetFieldId : ActorFieldId;
		const bool bIsConsumed =
			ResetPolicy == EMVFieldTransitionResetPolicy::PersistIfConsumed
			&& WorldState
			&& !ContextFieldId.IsNone()
			&& !ObjectId.IsNone()
			&& WorldState->IsOneTimeSpawnConsumed(ContextFieldId, ObjectId);

		FMVFieldTransitionResetContext ResetContext;
		ResetContext.FieldId = ContextFieldId;
		ResetContext.ObjectId = ObjectId;
		ResetContext.ResetPolicy = ResetPolicy;
		ResetContext.bIsConsumed = bIsConsumed;
		ResetContext.WorldState = WorldState;

		IMVFieldTransitionResettableInterface::Execute_HandleFieldTransitionReset(Actor, ResetContext);
		++ResetActorCount;
	}

	return ResetActorCount;
}

FName UMVFieldTransitionSubsystem::ResolveTransitionResetFieldId(const FMVFieldTransitionRequest& Request) const
{
	if (!Request.TargetFieldId.IsNone())
	{
		return Request.TargetFieldId;
	}

	if (!Request.bUseLastCheckpoint)
	{
		return NAME_None;
	}

	const UMVWorldStateSubsystem* WorldState = GetWorldState();
	if (!WorldState)
	{
		return NAME_None;
	}

	FMVCheckpointSaveData Checkpoint;
	if (!WorldState->TryGetLastCheckpoint(Checkpoint))
	{
		return NAME_None;
	}

	return Checkpoint.FieldId;
}

bool UMVFieldTransitionSubsystem::ApplyTransitionDestination(const FMVFieldTransitionRequest& Request)
{
	UWorld* World = GetWorld();
	AMVCharacterBase* Character = ResolvePlayerCharacter(World);
	if (!Character)
	{
		return false;
	}

	if (Request.bResetDeathPresentation && Character->DeathComponent)
	{
		Character->DeathComponent->ResetDeathPresentationForRespawn();
	}

	bool bHasDestination = Request.bHasTargetTransform;
	FTransform Destination = Request.TargetTransform;
	if (Request.bUseLastCheckpoint)
	{
		if (UMVWorldStateSubsystem* WorldState = GetWorldState())
		{
			FMVCheckpointSaveData Checkpoint;
			if (WorldState->TryGetLastCheckpoint(Checkpoint))
			{
				Destination = Checkpoint.Transform;
				bHasDestination = true;
			}
		}
	}

	if (bHasDestination)
	{
		Character->SetActorTransform(Destination, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (Request.bRestorePlayerStats)
	{
		ResetPlayerStatsForTransition(*Character);
	}

	return true;
}

void UMVFieldTransitionSubsystem::ResetPlayerStatsForTransition(AMVCharacterBase& Character) const
{
	UMVStatComponent* StatComponent = Character.StatComponent;
	if (!StatComponent)
	{
		return;
	}

	StatComponent->ResetDeathState();
	StatComponent->SetCurrentHP(StatComponent->MaxHP);
	StatComponent->SetCurrentStamina(StatComponent->MaxStamina);
	StatComponent->SetCurrentMP(StatComponent->MaxMP);
	StatComponent->SetCurrentGroggy(0.0f);

	if (UMVPlayerConsumableComponent* ConsumableComponent =
		Character.FindComponentByClass<UMVPlayerConsumableComponent>())
	{
		ConsumableComponent->RestoreHealingPotionCountForWorldReset();
	}
}

void UMVFieldTransitionSubsystem::ResetUIToDefaultAfterTransition()
{
	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->ResetToDefaultUI();
		ActiveLoadingWindow = nullptr;
	}
}

void UMVFieldTransitionSubsystem::RestorePlayerInputAfterTransition() const
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);
	PlayerController->ResetIgnoreMoveInput();
	PlayerController->ResetIgnoreLookInput();
}

void UMVFieldTransitionSubsystem::TryPreparePostTransitionAction(
	AMVCharacterBase& Character,
	const FMVFieldTransitionRequest& Request)
{
	PostTransitionActionComponent = nullptr;
	if (!Request.PostTransitionActionRow.DataTable || Request.PostTransitionActionRow.RowName.IsNone())
	{
		return;
	}

	if (UMVActionComponent* ActionComponent = Character.ActionComponent)
	{
		if (ActionComponent->IsActionRunning())
		{
			ActionComponent->CancelActiveAction(0.0f);
		}

		if (ActionComponent->TryStartActionFromRowHandle(
			Request.PostTransitionActionRow,
			Request.PostTransitionStartSection))
		{
			if (ActionComponent->PauseActiveAction())
			{
				PostTransitionActionComponent = ActionComponent;
			}
		}
	}
}

void UMVFieldTransitionSubsystem::ResumePreparedPostTransitionAction()
{
	if (PostTransitionActionComponent)
	{
		PostTransitionActionComponent->ResumeActiveAction();
		PostTransitionActionComponent = nullptr;
	}
}

AMVCharacterBase* UMVFieldTransitionSubsystem::ResolvePlayerCharacter(UWorld* World) const
{
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	return Cast<AMVCharacterBase>(Pawn);
}

UMVWorldStateSubsystem* UMVFieldTransitionSubsystem::GetWorldState() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMVWorldStateSubsystem>() : nullptr;
}

UMVUISubsystem* UMVFieldTransitionSubsystem::GetUISubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
}
