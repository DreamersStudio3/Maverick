#include "Cinematics/Boss/MVBossIntroTrigger.h"

#include "AI/Controller/MVAIController.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/MVActionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneObjectBindingID.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "System/MVWorldStateSubsystem.h"
#include "TimerManager.h"

AMVBossIntroTrigger::AMVBossIntroTrigger()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(120, 220, 150));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetHiddenInGame(true);
}

void AMVBossIntroTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlap);
	WorldState = UMVWorldStateSubsystem::Get(this);
	if (WorldState.IsValid()) WorldState->OnWorldStateLoaded.AddUniqueDynamic(this, &ThisClass::HandleWorldLoaded);
	// Pawn·Controller·StateTree의 BeginPlay 이후 AI 자동 시작도 대기 상태로 정리
	InitializeTimer = GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::InitializeEncounter);
}

void AMVBossIntroTrigger::InitializeEncounter()
{
	if (bEndingPlay || bInitialized || !IsValid(Boss) || IntroId.IsNone()) return;
	BossController = Cast<AMVAIController>(Boss->GetController());
	if (!BossController.IsValid()) return;
	InitialBossTransform = Boss->GetActorTransform();
	bInitialized = true;
	BossController->SetBossIntroBlocked(true);
	RestoreCompletedPlacement();
	TArray<AActor*> OverlappingActors;
	TriggerBox->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		if (TryStartIntro(Cast<APawn>(Actor))) break;
	}
}

void AMVBossIntroTrigger::RestoreCompletedPlacement()
{
	if (!IsValid(Boss) || !WorldState.IsValid()) return;
	FMVBossIntroSaveData Record;
	if (WorldState->TryGetBossIntroCompleted(IntroId, Record))
	{
		Boss->SetActorTransform(Record.BossTransform, false, nullptr, ETeleportType::TeleportPhysics);
		State = EMVBossIntroState::Completed;
	}
	else
	{
		Boss->SetActorTransform(InitialBossTransform, false, nullptr, ETeleportType::TeleportPhysics);
		State = EMVBossIntroState::Waiting;
	}
}

void AMVBossIntroTrigger::HandleWorldLoaded(FString SlotName)
{
	CancelIntro();
	bEncounterActivated = false;
	if (BossController.IsValid()) BossController->SetBossIntroBlocked(true);
	RestoreCompletedPlacement();
}

void AMVBossIntroTrigger::HandleOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryStartIntro(Cast<APawn>(OtherActor));
}

bool AMVBossIntroTrigger::TryStartIntro(APawn* PlayerPawn)
{
	if (bEndingPlay || IsIntroPlaying() || bEncounterActivated || !IsValid(PlayerPawn)
		|| !PlayerPawn->IsPlayerControlled()) return false;
	APlayerController* Controller = Cast<APlayerController>(PlayerPawn->GetController());
	if (!Controller || !Controller->IsLocalController()) return false;
	if (!bInitialized)
	{
		InitializeEncounter();
		if (IsIntroPlaying() || bEncounterActivated) return true;
	}
	if (!bInitialized || !WorldState.IsValid() || !IsValid(Boss)
		|| (Boss->StatComponent && Boss->StatComponent->IsDead())) return false;
	if (const AMVCharacterBase* Character = Cast<AMVCharacterBase>(PlayerPawn);
		Character && Character->StatComponent && Character->StatComponent->IsDead()) return false;
	for (TActorIterator<AMVBossIntroTrigger> It(GetWorld()); It; ++It)
	{
		if (*It != this && It->IsIntroPlaying()) return false;
	}
	RestoreCompletedPlacement();
	if (State == EMVBossIntroState::Completed)
	{
		ActivateCombat(PlayerPawn);
		return true;
	}
	if (!IntroSequence) return false;
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bPauseAtEnd = true;
	Settings.FinishCompletionStateOverride = EMovieSceneCompletionModeOverride::ForceRestoreState;
	ALevelSequenceActor* NewActor = nullptr;
	ULevelSequencePlayer* NewPlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(this, IntroSequence, Settings, NewActor);
	if (!NewPlayer || !NewActor)
	{
		if (NewActor) NewActor->Destroy();
		return false;
	}
	if (NewActor->FindNamedBindings(BossBindingTag).IsEmpty() || NewPlayer->GetDuration().AsSeconds() <= 0.0)
	{
		NewActor->Destroy();
		UE_LOG(LogTemp, Warning, TEXT("BossIntro: invalid duration / binding %s in %s"), *BossBindingTag.ToString(), *GetNameSafe(IntroSequence));
		return false;
	}
	SequencePlayer = NewPlayer;
	SequenceActor = NewActor;
	SequenceActor->SetBindingByTag(BossBindingTag, TArray<AActor*>{Boss}, false);
	SequencePlayer->OnFinished.AddUniqueDynamic(this, &ThisClass::HandleFinished);
	SequencePlayer->OnStop.AddUniqueDynamic(this, &ThisClass::HandleStopped);
	PlaybackStartTransform = Boss->GetActorTransform();
	SavedMovementMode = Boss->GetCharacterMovement()->MovementMode;
	SavedCustomMovementMode = Boss->GetCharacterMovement()->CustomMovementMode;
	bSavedCollision = Boss->GetActorEnableCollision();
	Boss->GetCharacterMovement()->StopMovementImmediately();
	Boss->GetCharacterMovement()->DisableMovement();
	Boss->SetActorEnableCollision(false);
	if (Boss->ActionComponent) Boss->ActionComponent->CancelActiveAction(0.0f);
	AcquirePlayerControl(PlayerPawn, Controller);
	State = EMVBossIntroState::Playing;
	SetActorTickEnabled(true);
	OnIntroStarted.Broadcast();
	if (IsIntroPlaying() && SequencePlayer) SequencePlayer->Play();
	return IsIntroPlaying();
}

void AMVBossIntroTrigger::AcquirePlayerControl(APawn* PlayerPawn, APlayerController* Controller)
{
	LocalPlayer = Controller;
	ControlledPawn = PlayerPawn;
	PreviousViewTarget = Controller->GetViewTarget();
	bSavedPawnInput = PlayerPawn->InputEnabled();
	bSavedCursor = Controller->bShowMouseCursor;
	bPlayerControlAcquired = true;
	if (AMVCharacterBase* Character = Cast<AMVCharacterBase>(PlayerPawn))
	{
		BlockedInput = Character->InputManagerComponent;
		if (BlockedInput.IsValid()) BlockedInput->BeginGameplayInputBlock();
		if (Character->ActionComponent) Character->ActionComponent->CancelActiveAction(0.0f);
		Character->GetCharacterMovement()->StopMovementImmediately();
	}
	Controller->SetIgnoreMoveInput(true);
	Controller->SetIgnoreLookInput(true);
	PlayerPawn->DisableInput(Controller);
	Controller->SetShowMouseCursor(false);
	if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
	{
		SavedCaptureMode = Viewport->GetMouseCaptureMode();
		SavedLockMode = Viewport->GetMouseLockMode();
		Controller->SetInputMode(FInputModeGameOnly());
		Viewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
		Viewport->SetMouseLockMode(EMouseLockMode::LockAlways);
	}
}

void AMVBossIntroTrigger::RestorePlayerControl()
{
	if (!bPlayerControlAcquired) return;
	bPlayerControlAcquired = false;
	if (BlockedInput.IsValid()) BlockedInput->EndGameplayInputBlock();
	if (APlayerController* Controller = LocalPlayer.Get())
	{
		Controller->SetIgnoreMoveInput(false);
		Controller->SetIgnoreLookInput(false);
		APawn* Pawn = ControlledPawn.Get();
		const bool bSamePawn = Pawn && Controller->GetPawn() == Pawn;
		const AMVCharacterBase* Character = Cast<AMVCharacterBase>(Pawn);
		if (bSamePawn && bSavedPawnInput && (!Character || !Character->StatComponent || !Character->StatComponent->IsDead())) Pawn->EnableInput(Controller);
		AActor* ViewTarget = bSamePawn ? PreviousViewTarget.Get() : Controller->GetPawn();
		if (IsValid(ViewTarget)) Controller->SetViewTarget(ViewTarget);
		Controller->SetShowMouseCursor(bSavedCursor);
		if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
		{
			if (bSavedCursor)
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetHideCursorDuringCapture(false);
				Controller->SetInputMode(InputMode);
			}
			else Controller->SetInputMode(FInputModeGameOnly());
			Viewport->SetMouseCaptureMode(SavedCaptureMode);
			Viewport->SetMouseLockMode(SavedLockMode);
		}
	}
	BlockedInput.Reset();
	PreviousViewTarget.Reset();
}

void AMVBossIntroTrigger::ActivateCombat(APawn* PlayerPawn)
{
	bEncounterActivated = true;
	if (BossController.IsValid())
	{
		BossController->SetBossIntroBlocked(false);
		BossController->SetBossCombatEnabled(true, PlayerPawn);
	}
}

void AMVBossIntroTrigger::HandleFinished() { FinishIntro(true); }
void AMVBossIntroTrigger::HandleStopped() { FinishIntro(false); }
void AMVBossIntroTrigger::CancelIntro() { FinishIntro(false); }

void AMVBossIntroTrigger::FinishIntro(const bool bCompleted)
{
	if (!IsIntroPlaying()) return;
	const bool bBossAlive = IsValid(Boss) && (!Boss->StatComponent || !Boss->StatComponent->IsDead());
	const bool bSuccess = bCompleted && bBossAlive;
	// bPauseAtEnd로 마지막 평가 위치 확보, Stop의 상태 복원 이후 캡슐 위치 재적용
	const FTransform FinalTransform = bSuccess ? Boss->GetActorTransform() : PlaybackStartTransform;
	State = bSuccess ? EMVBossIntroState::Completed : EMVBossIntroState::Waiting;
	SetActorTickEnabled(false);
	if (SequencePlayer)
	{
		SequencePlayer->OnFinished.RemoveAll(this);
		SequencePlayer->OnStop.RemoveAll(this);
		SequencePlayer->Stop();
	}
	if (SequenceActor) SequenceActor->Destroy();
	SequencePlayer = nullptr;
	SequenceActor = nullptr;
	if (bBossAlive)
	{
		Boss->SetActorTransform(FinalTransform, false, nullptr, ETeleportType::TeleportPhysics);
		Boss->SetActorEnableCollision(bSavedCollision);
		Boss->GetCharacterMovement()->SetMovementMode(static_cast<EMovementMode>(SavedMovementMode), SavedCustomMovementMode);
	}
	RestorePlayerControl();
	if (bSuccess && WorldState.IsValid())
	{
		WorldState->RecordBossIntroCompleted(IntroId, FinalTransform);
		if (!WorldState->SaveToSlot(WorldState->ActiveSaveSlotName, WorldState->ActiveUserIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("BossIntro: save failed for %s, completion retained in memory"), *IntroId.ToString());
		}
		ActivateCombat(ControlledPawn.Get());
	}
	ControlledPawn.Reset();
	LocalPlayer.Reset();
	OnIntroEnded.Broadcast(bSuccess);
}

void AMVBossIntroTrigger::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const AMVCharacterBase* Character = Cast<AMVCharacterBase>(ControlledPawn.Get());
	if (!IsValid(Boss) || !ControlledPawn.IsValid() || !LocalPlayer.IsValid()
		|| LocalPlayer->GetPawn() != ControlledPawn.Get()
		|| (Boss->StatComponent && Boss->StatComponent->IsDead())
		|| (Character && Character->StatComponent && Character->StatComponent->IsDead()))
	{
		CancelIntro();
	}
}

void AMVBossIntroTrigger::EndPlay(const EEndPlayReason::Type Reason)
{
	bEndingPlay = true;
	GetWorldTimerManager().ClearTimer(InitializeTimer);
	CancelIntro();
	TriggerBox->OnComponentBeginOverlap.RemoveAll(this);
	if (WorldState.IsValid()) WorldState->OnWorldStateLoaded.RemoveAll(this);
	if (BossController.IsValid()) BossController->SetBossIntroBlocked(false);
	Super::EndPlay(Reason);
}
