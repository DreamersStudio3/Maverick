#include "System/MVFieldTransitionSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Character/PC/Consumable/MVPlayerConsumable.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVActionComponent.h"
#include "Components/MVDeathComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/RestPoint/MVRestPointSubsystem.h"
#include "Interaction/RestPoint/MVRestPoint.h"
#include "Kismet/GameplayStatics.h"
#include "System/MVDeathRespawnFlow.h"
#include "System/MVFieldEnemySpawnPoint.h"
#include "System/MVFieldTransitionSettings.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/MVWorldStateTypes.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/System/MVUISettings.h"
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

bool MVFieldTransitionTryPlaceAtCheckpoint(UWorld& World, AMVCharacterBase& Character, const FTransform& Transform)
{
	FVector Location = Transform.GetLocation();
	const FRotator Rotation = Transform.Rotator();
	return Transform.IsValid() && World.FindTeleportSpot(&Character, Location, Rotation)
		&& Character.SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
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
	if (TransitionPhase != EMVFieldTransitionPhase::Idle
		|| (Request.Reason == EMVFieldTransitionReason::Rest && Request.RestRequestId == 0))
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
	ActiveLoadingWindow = nullptr;
	if (UISubsystem)
	{
		if (ActiveTransitionRequest.Reason == EMVFieldTransitionReason::Rest)
		{
			const UMVUISettings* Settings = GetDefault<UMVUISettings>();
			ActiveLoadingWindow = Cast<UMVLoadingWindow>(UISubsystem->PushWindowByClass(Settings->LoadingWindowClass));
		}
		else
		{
			ActiveLoadingWindow = Cast<UMVLoadingWindow>(UISubsystem->ShowLoadingWindow());
		}
	}

	UpdateTransitionProgress(0.15f, NSLOCTEXT("MaverickFieldTransition", "LoadingPrepare", "필드 전환 준비 중"));
	UpdateTransitionProgress(0.45f, ActiveTransitionRequest.Reason == EMVFieldTransitionReason::CheckpointTravel
		? NSLOCTEXT("MaverickFieldTransition", "FieldResetSkipped", "이동 위치 확인 중")
		: NSLOCTEXT("MaverickFieldTransition", "FieldReset", "필드 초기화 준비 중"));
	UpdateTransitionProgress(0.75f, NSLOCTEXT("MaverickFieldTransition", "WorldState", "저장 상태 적용 준비"));
	StartAutomaticLoadingCompletion();
}

bool UMVFieldTransitionSubsystem::StartRestTransition(AMVRestPoint& Point, const uint64 RequestId)
{
	UWorld* World = GetWorld();
	AMVCharacterBase* Character = ResolvePlayerCharacter(World);
	const FTransform Anchor = Point.GetRespawnTransform();
	FVector SafeLocation = Anchor.GetLocation();
	if (!World || !Character || Point.GetWorld() != World || IsTransitionRunning()
		|| !Anchor.IsValid() || !World->FindTeleportSpot(Character, SafeLocation, Anchor.Rotator())
		|| !AMVFieldEnemySpawnPoint::CanResetField(*World, Point.GetFieldId()))
	{
		return false;
	}

	FMVFieldTransitionRequest Request;
	Request.Reason = EMVFieldTransitionReason::Rest;
	Request.TargetFieldId = Point.GetFieldId();
	Request.RestRequestId = RequestId;
	Request.bClearUIBeforeLoading = false;
	Request.bResetUIToDefaultOnComplete = false;
	Request.bRestoreInputOnComplete = false;
	return StartTransition(Request);
}

bool UMVFieldTransitionSubsystem::IsRestTransition(const uint64 RequestId) const
{
	return RequestId != 0 && ActiveTransitionRequest.Reason == EMVFieldTransitionReason::Rest
		&& ActiveTransitionRequest.RestRequestId == RequestId && IsTransitionRunning();
}

bool UMVFieldTransitionSubsystem::StartDeathRespawnTransition(AActor* DeadActor)
{
	FMVFieldTransitionRequest Request;
	Request.Reason = EMVFieldTransitionReason::DeathRespawn;
	Request.SourceActor = DeadActor;
	Request.bUseLastCheckpoint = true;
	Request.bResetDeathPresentation = true;
	Request.bRestorePlayerStats = true;
	Request.bClearUIBeforeLoading = false;
	Request.bResetUIToDefaultOnComplete = true;
	Request.bRestoreInputOnComplete = true;
	if (UWorld* World = GetWorld())
	{
		FName PreferredField;
		if (const UMVWorldStateSubsystem* State = GetWorldState())
		{
			const FMVWorldSaveData& Save = State->GetSaveData();
			const UMVRestPointSubsystem* Rest = World->GetSubsystem<UMVRestPointSubsystem>();
			FTransform Anchor;
			if (Rest && Rest->TryResolveRespawnAnchor(Save.LastRestPoint, Anchor))
			{
				PreferredField = Save.LastRestPoint.FieldId;
			}
			else if (Save.LastCheckpoint.bHasCheckpoint && (Save.LastCheckpoint.MapName.IsNone()
				|| Save.LastCheckpoint.MapName == FName(*UGameplayStatics::GetCurrentLevelName(World, true))))
			{
				PreferredField = Save.LastCheckpoint.FieldId;
			}
		}
		Request.TargetFieldId = AMVFieldEnemySpawnPoint::ResolveDeathResetField(*World, PreferredField);
	}
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
			|| !WorldState->SaveActiveSlot())
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
	if (TransitionPhase != EMVFieldTransitionPhase::LoadingReset)
	{
		return;
	}
	if (ActiveTransitionRequest.Reason == EMVFieldTransitionReason::Rest
		&& ActiveLoadingWindow && !ActiveLoadingWindow->IsActivated())
	{
		// 같은 거점 스택의 진입 전환 전에 닫으면 나중에 로딩창이 다시 활성화된다.
		ActiveLoadingWindow->OnActivated().RemoveAll(this);
		ActiveLoadingWindow->OnActivated().AddWeakLambda(this, [this]()
		{
			if (UWorld* World = GetWorld())
			{
				AutomaticLoadingTimerHandle = World->GetTimerManager().SetTimerForNextTick(
					this, &UMVFieldTransitionSubsystem::CompleteTransition);
			}
		});
		return;
	}
	if (ActiveLoadingWindow)
	{
		ActiveLoadingWindow->OnActivated().RemoveAll(this);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutomaticLoadingTimerHandle);
		World->GetTimerManager().ClearTimer(ScreenTransitionCompletionTimerHandle);
	}

	SetTransitionPhase(EMVFieldTransitionPhase::Relocating);
	if (ActiveTransitionRequest.Reason == EMVFieldTransitionReason::Rest)
	{
		bRestApplied = ApplyRestTransition();
		UpdateTransitionProgress(1.0f, bRestApplied
			? NSLOCTEXT("MaverickFieldTransition", "RestApplied", "회복 완료")
			: NSLOCTEXT("MaverickFieldTransition", "RestFailed", "휴식을 완료하지 못했습니다"));
	}
	else
	{
		UWorld* World = GetWorld();
		AMVCharacterBase* Character = ResolvePlayerCharacter(World);
		if (World && ActiveTransitionRequest.Reason == EMVFieldTransitionReason::DeathRespawn
			&& !AMVFieldEnemySpawnPoint::ResetField(*World, ActiveTransitionRequest.TargetFieldId, Character))
		{
			UE_LOG(LogMVFieldTransitionSubsystem, Warning, TEXT("Enemy reset incomplete during death respawn."));
		}
		UpdateTransitionProgress(1.0f, NSLOCTEXT("MaverickFieldTransition", "Complete", "전환 위치 적용"));
		const bool bDestinationApplied = ApplyTransitionDestination(ActiveTransitionRequest);
		if (Character && bDestinationApplied)
		{
			TryPreparePostTransitionAction(*Character, ActiveTransitionRequest);
		}
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

	const uint64 RestRequestId = ActiveTransitionRequest.RestRequestId;
	const bool bApplied = bRestApplied;
	ResetTransitionState();
	SetTransitionPhase(EMVFieldTransitionPhase::Idle);
	if (UWorld* World = GetWorld(); World && RestRequestId != 0)
	{
		if (UMVRestPointSubsystem* Rest = World->GetSubsystem<UMVRestPointSubsystem>())
		{
			Rest->FinishRestRequest(RestRequestId, bApplied);
		}
	}
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

	return !bSaveImmediately || WorldState->SaveActiveSlot();
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
	if (!bCompleteLoadingAutomatically && ActiveTransitionRequest.Reason != EMVFieldTransitionReason::Rest)
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
	if (ActiveLoadingWindow)
	{
		ActiveLoadingWindow->OnActivated().RemoveAll(this);
	}
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
	bRestApplied = false;
}

bool UMVFieldTransitionSubsystem::ApplyRestTransition()
{
	UWorld* World = GetWorld();
	UMVRestPointSubsystem* Rest = World ? World->GetSubsystem<UMVRestPointSubsystem>() : nullptr;
	AMVCharacterBase* Character = ResolvePlayerCharacter(World);
	const uint64 RequestId = ActiveTransitionRequest.RestRequestId;
	if (!Rest || !Character || !Rest->IsRestRequestValid(RequestId)
		|| !AMVFieldEnemySpawnPoint::ResetField(*World, ActiveTransitionRequest.TargetFieldId, Character)
		|| !IsValid(Character) || !Rest->IsRestRequestValid(RequestId))
	{
		return false;
	}

	ResetPlayerStatsForTransition(*Character);
	return true;
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

	bool bAppliedDestination = false;
	if (Request.bUseLastCheckpoint)
	{
		if (const UMVWorldStateSubsystem* WorldState = GetWorldState())
		{
			const FMVWorldSaveData& Save = WorldState->GetSaveData();
			const FMVCheckpointSaveData& RestPoint = Save.LastRestPoint;
			const FMVCheckpointSaveData& Checkpoint = Save.LastCheckpoint;
			if (RestPoint.bHasCheckpoint)
			{
				FTransform Anchor;
				const UMVRestPointSubsystem* Rest = World->GetSubsystem<UMVRestPointSubsystem>();
				bAppliedDestination = Rest && Rest->TryResolveRespawnAnchor(RestPoint, Anchor)
					&& MVFieldTransitionTryPlaceAtCheckpoint(*World, *Character, Anchor);
			}
			else if (Checkpoint.bHasCheckpoint && (Checkpoint.MapName.IsNone()
				|| Checkpoint.MapName == FName(*UGameplayStatics::GetCurrentLevelName(World, true))))
			{
				bAppliedDestination = MVFieldTransitionTryPlaceAtCheckpoint(*World, *Character, Checkpoint.Transform);
			}
		}

		if (!bAppliedDestination)
		{
			AGameModeBase* GameMode = World->GetAuthGameMode();
			AActor* Start = GameMode ? GameMode->FindPlayerStart(Character->GetController()) : nullptr;
			bAppliedDestination = Start && MVFieldTransitionTryPlaceAtCheckpoint(*World, *Character, Start->GetActorTransform());
			if (!bAppliedDestination)
			{
				UE_LOG(LogMVFieldTransitionSubsystem, Warning, TEXT("No usable respawn destination in %s."), *World->GetMapName());
			}
		}
	}
	else if (Request.bHasTargetTransform)
	{
		bAppliedDestination = Character->SetActorTransform(Request.TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (Request.bRestorePlayerStats)
	{
		ResetPlayerStatsForTransition(*Character);
	}

	return bAppliedDestination || (!Request.bUseLastCheckpoint && !Request.bHasTargetTransform);
}

void UMVFieldTransitionSubsystem::ResetPlayerStatsForTransition(AMVCharacterBase& Character) const
{
	UMVStatComponent* StatComponent = Character.StatComponent;
	if (!StatComponent)
	{
		return;
	}

	StatComponent->ResetDeathState();
	ResetPlayerCombatForField(Character);
	StatComponent->SetCurrentHP(StatComponent->MaxHP);
	StatComponent->SetCurrentStamina(StatComponent->MaxStamina);
	StatComponent->SetCurrentMP(StatComponent->MaxMP);
	StatComponent->ResetCombatDamageForWorldReset();

	AMVPlayerCharacter* PlayerCharacter = Cast<AMVPlayerCharacter>(&Character);
	if (UMVPlayerConsumable* Consumable =
		PlayerCharacter ? PlayerCharacter->PlayerConsumable : nullptr)
	{
		Consumable->RestoreConsumableCountsForWorldReset();
	}
}

void UMVFieldTransitionSubsystem::ResetPlayerCombatForField(AMVCharacterBase& Character) const
{
	UMVCombatComponent* Combat = Character.FindComponentByClass<UMVCombatComponent>();
	UMVStatComponent* Stats = Character.StatComponent.Get();
	Stats->BeginMaxHPModifierBatch();
	if (Combat)
	{
		Combat->ClearPassiveSkill();
	}
	if (UMVStatusEffectComponent* Effects = Character.StatusEffectComponent.Get())
	{
		TArray<FMVStatusEffectHandle> TimedEffects;
		for (const FMVStatusEffectInstance& Effect : Effects->GetActiveEffects())
		{
			if (Effect.HasExpiration())
			{
				TimedEffects.Add(Effect.Handle);
			}
		}
		for (const FMVStatusEffectHandle Handle : TimedEffects)
		{
			Effects->RemoveStatusEffect(Handle, EMVStatusEffectRemovalReason::Cleared);
		}
	}
	if (Combat)
	{
		Combat->RefreshSkillSlot(EMVSkillSlot::PassiveSkill);
	}
	Stats->EndMaxHPModifierBatch();
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
