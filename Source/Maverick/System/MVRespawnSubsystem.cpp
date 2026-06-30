#include "System/MVRespawnSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVDeathComponent.h"
#include "Components/MVStatComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "System/MVRespawnResettableInterface.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/MVWorldStateTypes.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVDeathOverlayWindow.h"
#include "UI/Window/MVLoadingWindow.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVRespawnSubsystem, Log, All);

void UMVRespawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UMVWorldStateSubsystem::StaticClass());

	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMVRespawnSubsystem::HandleWorldInit);
}

void UMVRespawnSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	ResetRespawnState();

	Super::Deinitialize();
}

UMVRespawnSubsystem* UMVRespawnSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVRespawnSubsystem>() : nullptr;
}

bool UMVRespawnSubsystem::BeginDeathSequence(AActor* DeadActor)
{
	if (RespawnPhase != EMVRespawnPhase::Idle)
	{
		return false;
	}

	PendingDeadActor = DeadActor;
	bDeathMontageEnded = false;
	bDeathOverlayMinimumElapsed = false;
	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->ClearAllUI(true);
		ActiveDeathOverlay = nullptr;
		ActiveLoadingWindow = nullptr;
		UE_LOG(
			LogMVRespawnSubsystem,
			Display,
			TEXT("[DeathFlowTest] Death.ClearAllUI"));
	}

	SetRespawnPhase(EMVRespawnPhase::DeathStarted);
	UpdateRespawnProgress(0.0f, NSLOCTEXT("MaverickRespawn", "DeathStarted", "사망 처리 시작"));
	return true;
}

void UMVRespawnSubsystem::NotifyDeathOverlayRequested()
{
	if (RespawnPhase == EMVRespawnPhase::LoadingReset || RespawnPhase == EMVRespawnPhase::Respawning)
	{
		return;
	}

	if (RespawnPhase == EMVRespawnPhase::Idle)
	{
		PendingDeadActor = ResolvePlayerCharacter(GetWorld());
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			UISubsystem->ClearAllUI(true);
			ActiveDeathOverlay = nullptr;
			ActiveLoadingWindow = nullptr;
			UE_LOG(
				LogMVRespawnSubsystem,
				Display,
				TEXT("[DeathFlowTest] Death.ClearAllUI"));
		}

		SetRespawnPhase(EMVRespawnPhase::DeathStarted);
	}

	SetRespawnPhase(EMVRespawnPhase::DeathOverlay);
	UpdateRespawnProgress(0.05f, NSLOCTEXT("MaverickRespawn", "DeathOverlay", "사망 알림 표시"));

	if (ActiveDeathOverlay && ActiveDeathOverlay->IsActivated())
	{
		return;
	}

	UMVUISubsystem* UISubsystem = GetUISubsystem();
	ActiveDeathOverlay = UISubsystem
		? Cast<UMVDeathOverlayWindow>(UISubsystem->ShowDeathOverlay())
		: nullptr;
	if (ActiveDeathOverlay)
	{
		ActiveDeathOverlay->OnMinimumDisplayElapsed.RemoveDynamic(
			this,
			&UMVRespawnSubsystem::HandleDeathOverlayMinimumDisplayElapsed);
		ActiveDeathOverlay->OnMinimumDisplayElapsed.AddDynamic(
			this,
			&UMVRespawnSubsystem::HandleDeathOverlayMinimumDisplayElapsed);
	}
}

void UMVRespawnSubsystem::NotifyDeathDissolveStarted()
{
	NotifyDeathOverlayRequested();
}

void UMVRespawnSubsystem::NotifyDeathMontageEnded()
{
	if (RespawnPhase == EMVRespawnPhase::Idle)
	{
		PendingDeadActor = ResolvePlayerCharacter(GetWorld());
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			UISubsystem->ClearAllUI(true);
			ActiveDeathOverlay = nullptr;
			ActiveLoadingWindow = nullptr;
			UE_LOG(
				LogMVRespawnSubsystem,
				Display,
				TEXT("[DeathFlowTest] Death.ClearAllUI"));
		}

		SetRespawnPhase(EMVRespawnPhase::DeathStarted);
	}

	bDeathMontageEnded = true;
	if (bDeathOverlayMinimumElapsed)
	{
		BeginRespawnLoading();
		return;
	}

	if (!ActiveDeathOverlay)
	{
		NotifyDeathOverlayRequested();
	}
}

void UMVRespawnSubsystem::BeginRespawnLoading()
{
	if (RespawnPhase == EMVRespawnPhase::LoadingReset || RespawnPhase == EMVRespawnPhase::Respawning)
	{
		return;
	}

	SetRespawnPhase(EMVRespawnPhase::LoadingReset);

	if (ActiveDeathOverlay)
	{
		ActiveDeathOverlay->OnMinimumDisplayElapsed.RemoveDynamic(
			this,
			&UMVRespawnSubsystem::HandleDeathOverlayMinimumDisplayElapsed);
		ActiveDeathOverlay->DeactivateWidget();
		ActiveDeathOverlay = nullptr;
	}

	UMVUISubsystem* UISubsystem = GetUISubsystem();
	ActiveLoadingWindow = UISubsystem
		? Cast<UMVLoadingWindow>(UISubsystem->ShowLoadingWindow())
		: nullptr;

	UpdateRespawnProgress(0.15f, NSLOCTEXT("MaverickRespawn", "RespawnLoadingPrepare", "부활 준비 중"));
	const int32 ResetActorCount = ResetWorldActorsForRespawn();
	UpdateRespawnProgress(
		0.45f,
		FText::Format(
			NSLOCTEXT("MaverickRespawn", "RespawnLoadingFieldReset", "필드 상태 초기화 대상 {0}개 적용"),
			FText::AsNumber(ResetActorCount)));
	UpdateRespawnProgress(0.75f, NSLOCTEXT("MaverickRespawn", "RespawnLoadingWorldState", "저장 상태 적용 준비"));
	StartAutomaticLoadingCompletion();
}

void UMVRespawnSubsystem::CompleteRespawn()
{
	if (RespawnPhase == EMVRespawnPhase::Idle)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutomaticLoadingTimerHandle);
	}

	SetRespawnPhase(EMVRespawnPhase::Respawning);
	UpdateRespawnProgress(1.0f, NSLOCTEXT("MaverickRespawn", "RespawnComplete", "부활 위치 적용"));
	RespawnPlayerAtLastCheckpoint();

	if (ActiveLoadingWindow)
	{
		ActiveLoadingWindow->DeactivateWidget();
	}

	if (ActiveDeathOverlay)
	{
		ActiveDeathOverlay->OnMinimumDisplayElapsed.RemoveDynamic(
			this,
			&UMVRespawnSubsystem::HandleDeathOverlayMinimumDisplayElapsed);
		ActiveDeathOverlay->DeactivateWidget();
	}

	ResetUIToDefaultAfterRespawn();
	RestorePlayerInputAfterRespawn();
	ResetRespawnState();
	SetRespawnPhase(EMVRespawnPhase::Idle);
}

bool UMVRespawnSubsystem::SetLastCheckpointFromActor(
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

void UMVRespawnSubsystem::HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	FTimerHandle PostWorldInitTimerHandle;
	const TWeakObjectPtr<UWorld> WeakWorld = World;
	World->GetTimerManager().SetTimer(
		PostWorldInitTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
		{
			UWorld* CurrentWorld = WeakWorld.Get();
			if (CurrentWorld && CurrentWorld->IsGameWorld())
			{
				BindToPlayerDeath(CurrentWorld);
			}
		}),
		0.1f,
		false);
}

void UMVRespawnSubsystem::BindToPlayerDeath(UWorld* World)
{
	AMVCharacterBase* Character = ResolvePlayerCharacter(World);
	UMVDeathComponent* DeathComponent = Character ? Character->DeathComponent.Get() : nullptr;
	if (!DeathComponent)
	{
		return;
	}

	DeathComponent->OnDeathPresentationStarted.RemoveDynamic(
		this,
		&UMVRespawnSubsystem::HandlePlayerDeathPresentationStarted);
	DeathComponent->OnDeathPresentationStarted.AddUniqueDynamic(
		this,
		&UMVRespawnSubsystem::HandlePlayerDeathPresentationStarted);
	DeathComponent->OnDeathOverlayRequested.RemoveDynamic(
		this,
		&UMVRespawnSubsystem::HandlePlayerDeathOverlayRequested);
	DeathComponent->OnDeathOverlayRequested.AddUniqueDynamic(
		this,
		&UMVRespawnSubsystem::HandlePlayerDeathOverlayRequested);
	DeathComponent->OnDeathPresentationFinished.RemoveDynamic(
		this,
		&UMVRespawnSubsystem::HandlePlayerDeathPresentationFinished);
	DeathComponent->OnDeathPresentationFinished.AddUniqueDynamic(
		this,
		&UMVRespawnSubsystem::HandlePlayerDeathPresentationFinished);
}

void UMVRespawnSubsystem::SetRespawnPhase(const EMVRespawnPhase NewPhase)
{
	if (RespawnPhase == NewPhase)
	{
		return;
	}

	RespawnPhase = NewPhase;
	OnRespawnPhaseChanged.Broadcast(RespawnPhase);
}

void UMVRespawnSubsystem::UpdateRespawnProgress(const float Progress, FText StepText)
{
	const float ClampedProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	if (ActiveLoadingWindow)
	{
		ActiveLoadingWindow->SetLoadingProgress(ClampedProgress, StepText);
	}

	OnRespawnProgressChanged.Broadcast(ClampedProgress, StepText);
}

void UMVRespawnSubsystem::StartAutomaticLoadingCompletion()
{
	if (!bCompleteLoadingAutomatically)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		CompleteRespawn();
		return;
	}

	const float DelaySeconds = FMath::Max(0.0f, MinimumLoadingSeconds);
	if (DelaySeconds <= 0.0f)
	{
		CompleteRespawn();
		return;
	}

	World->GetTimerManager().SetTimer(
		AutomaticLoadingTimerHandle,
		this,
		&UMVRespawnSubsystem::CompleteRespawn,
		DelaySeconds,
		false);
}

void UMVRespawnSubsystem::ResetRespawnState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutomaticLoadingTimerHandle);
	}

	PendingDeadActor.Reset();
	ActiveDeathOverlay = nullptr;
	ActiveLoadingWindow = nullptr;
	bDeathMontageEnded = false;
	bDeathOverlayMinimumElapsed = false;
}

int32 UMVRespawnSubsystem::ResetWorldActorsForRespawn()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	// 현재 체크포인트 필드를 기준으로 리셋 가능한 월드 actor에게 사망 리셋 context를 전달한다.
	UMVWorldStateSubsystem* WorldState = GetWorldState();
	const FName TargetFieldId = ResolveRespawnResetFieldId();
	int32 ResetActorCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		// 사망한 플레이어 본체와 유효하지 않은 actor는 리셋 대상에서 제외한다.
		if (!IsValid(Actor) || Actor == PendingDeadActor.Get())
		{
			continue;
		}

		// 리셋 정책은 actor가 명시적으로 인터페이스를 구현한 경우에만 위임한다.
		if (!Actor->GetClass()->ImplementsInterface(UMVRespawnResettableInterface::StaticClass()))
		{
			continue;
		}

		// actor가 특정 FieldId를 선언했다면 마지막 체크포인트 필드와 일치할 때만 리셋한다.
		const FName ActorFieldId = IMVRespawnResettableInterface::Execute_GetRespawnResetFieldId(Actor);
		if (!TargetFieldId.IsNone() && !ActorFieldId.IsNone() && ActorFieldId != TargetFieldId)
		{
			continue;
		}

		// 1회성 대상은 WorldState 저장 데이터를 조회해 이미 소비됐는지 context에 담는다.
		const EMVRespawnResetPolicy ResetPolicy =
			IMVRespawnResettableInterface::Execute_GetRespawnResetPolicy(Actor);
		const FName ObjectId = IMVRespawnResettableInterface::Execute_GetRespawnResetObjectId(Actor);
		const FName ContextFieldId = ActorFieldId.IsNone() ? TargetFieldId : ActorFieldId;
		const bool bIsConsumed =
			ResetPolicy == EMVRespawnResetPolicy::PersistIfConsumed
			&& WorldState
			&& !ContextFieldId.IsNone()
			&& !ObjectId.IsNone()
			&& WorldState->IsOneTimeSpawnConsumed(ContextFieldId, ObjectId);

		FMVRespawnResetContext ResetContext;
		ResetContext.FieldId = ContextFieldId;
		ResetContext.ObjectId = ObjectId;
		ResetContext.ResetPolicy = ResetPolicy;
		ResetContext.bIsConsumed = bIsConsumed;
		ResetContext.WorldState = WorldState;

		// 실제 초기화, 숨김, 제거, 영구 상태 재적용은 각 actor 구현이 담당한다.
		IMVRespawnResettableInterface::Execute_HandleRespawnReset(Actor, ResetContext);
		++ResetActorCount;
	}

	return ResetActorCount;
}

FName UMVRespawnSubsystem::ResolveRespawnResetFieldId() const
{
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

bool UMVRespawnSubsystem::RespawnPlayerAtLastCheckpoint()
{
	UWorld* World = GetWorld();
	AMVCharacterBase* Character = ResolvePlayerCharacter(World);
	if (!Character)
	{
		return false;
	}

	if (Character->DeathComponent)
	{
		Character->DeathComponent->ResetDeathPresentationForRespawn();
	}

	if (UMVWorldStateSubsystem* WorldState = GetWorldState())
	{
		FMVCheckpointSaveData Checkpoint;
		if (WorldState->TryGetLastCheckpoint(Checkpoint))
		{
			Character->SetActorTransform(Checkpoint.Transform, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	ResetPlayerStatsForRespawn(*Character);
	return true;
}

void UMVRespawnSubsystem::ResetPlayerStatsForRespawn(AMVCharacterBase& Character) const
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
}

void UMVRespawnSubsystem::ResetUIToDefaultAfterRespawn()
{
	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->ResetToDefaultUI();
		ActiveDeathOverlay = nullptr;
		ActiveLoadingWindow = nullptr;
		UE_LOG(
			LogMVRespawnSubsystem,
			Display,
			TEXT("[DeathFlowTest] Respawn.UIResetToDefault"));
	}
}

void UMVRespawnSubsystem::RestorePlayerInputAfterRespawn() const
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

	UE_LOG(
		LogMVRespawnSubsystem,
		Display,
		TEXT("[DeathFlowTest] Respawn.InputRestored Controller=%s"),
		*GetNameSafe(PlayerController));
}

AMVCharacterBase* UMVRespawnSubsystem::ResolvePlayerCharacter(UWorld* World) const
{
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	return Cast<AMVCharacterBase>(Pawn);
}

UMVWorldStateSubsystem* UMVRespawnSubsystem::GetWorldState() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMVWorldStateSubsystem>() : nullptr;
}

UMVUISubsystem* UMVRespawnSubsystem::GetUISubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
}

void UMVRespawnSubsystem::HandlePlayerDeathPresentationStarted(AActor* DeadActor)
{
	BeginDeathSequence(DeadActor);
}

void UMVRespawnSubsystem::HandlePlayerDeathOverlayRequested(AActor* /*DeadActor*/)
{
	NotifyDeathOverlayRequested();
}

void UMVRespawnSubsystem::HandlePlayerDeathPresentationFinished(AActor* /*DeadActor*/)
{
	NotifyDeathMontageEnded();
}

void UMVRespawnSubsystem::HandleDeathOverlayMinimumDisplayElapsed()
{
	bDeathOverlayMinimumElapsed = true;
	if (bDeathMontageEnded)
	{
		BeginRespawnLoading();
	}
}
