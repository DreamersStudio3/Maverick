#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "AI/Controller/MVAIController.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Cinematics/Boss/MVBossIntroTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieScene.h"
#include "MovieSceneObjectBindingID.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "System/MVWorldStateSubsystem.h"
#include "Misc/ScopeExit.h"
#include "TimerManager.h"

namespace MaverickBossIntroTestsPrivate
{
ULevelSequence* MakeSequence(AMVEnemy* Boss, UWorld* World)
{
	ULevelSequence* Sequence = NewObject<ULevelSequence>();
	Sequence->Initialize();
	UMovieScene* Scene = Sequence->GetMovieScene();
	Scene->SetTickResolutionDirectly(FFrameRate(30, 1));
	Scene->SetDisplayRate(FFrameRate(30, 1));
	Scene->SetPlaybackRange(0, 30);
	const FGuid Guid = Scene->AddPossessable(TEXT("Boss"), AMVEnemy::StaticClass());
	Sequence->BindPossessableObject(Guid, *Boss, World);
	Scene->TagBinding(TEXT("Boss"), UE::MovieScene::FFixedObjectBindingID(Guid, MovieSceneSequenceID::Root));
	UMovieScene3DTransformTrack* Track = Scene->AddTrack<UMovieScene3DTransformTrack>(Guid);
	UMovieScene3DTransformSection* Section = Cast<UMovieScene3DTransformSection>(Track->CreateNewSection());
	Section->SetRange(TRange<FFrameNumber>(0, 30));
	Section->SetCompletionMode(EMovieSceneCompletionMode::RestoreState);
	Track->AddSection(*Section);
	TArrayView<FMovieSceneDoubleChannel*> Channels = Section->GetChannelProxy().GetChannels<FMovieSceneDoubleChannel>();
	for (int32 Index = 0; Index < Channels.Num(); ++Index) Channels[Index]->SetDefault(Index >= 6 ? 1.0 : 0.0);
	Channels[0]->AddLinearKey(0, Boss->GetActorLocation().X);
	Channels[0]->AddLinearKey(29, 800.0);
	Channels[1]->SetDefault(200.0);
	Channels[2]->SetDefault(1000.0);
	return Sequence;
}

ULevelSequencePlayer* FindPlayer(UWorld* World)
{
	for (TActorIterator<ALevelSequenceActor> It(World); It; ++It)
	{
		if (!It->IsActorBeingDestroyed()) return It->GetSequencePlayer();
	}
	return nullptr;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVBossIntroTest, "Maverick.Cinematics.BossIntro",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVBossIntroTest::RunTest(const FString& Parameters)
{
	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	GI->InitializeStandalone();
	UWorld* PreviousWorld = GI->GetWorld();
	const UWorld::InitializationValues Values = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
	World->SetGameInstance(GI);
	GI->GetWorldContext()->SetCurrentWorld(World);
	if (PreviousWorld && PreviousWorld != World) PreviousWorld->DestroyWorld(false);
	UMVWorldStateSubsystem* State = GI->GetSubsystem<UMVWorldStateSubsystem>();
	State->ResetSaveData();
	const FString Slot = TEXT("Automation_BossIntro_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	State->ActiveSaveSlotName = Slot;
	State->ActiveUserIndex = 0;
	ON_SCOPE_EXIT
	{
		State->DeleteSaveSlot(Slot, 0);
		TArray<AActor*> Actors;
		for (TActorIterator<AActor> It(World); It; ++It) Actors.Add(*It);
		for (AActor* Actor : Actors) World->DestroyActor(Actor, true);
		World->DestroyWorld(false);
		GI->Shutdown();
		GEngine->DestroyWorldContext(World);
	};
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMVEnemy* Boss = World->SpawnActor<AMVEnemy>(FVector(300, 200, 1000), FRotator::ZeroRotator, Spawn);
	AMVAIController* AI = World->SpawnActor<AMVAIController>();
	AI->Possess(Boss);
	Boss->DispatchBeginPlay();
	AI->DispatchBeginPlay();
	AMVCharacterBase* Pawn = World->SpawnActor<AMVCharacterBase>(FVector(-500, -500, 1000), FRotator::ZeroRotator, Spawn);
	APlayerController* PC = World->SpawnActor<APlayerController>();
	PC->SetPlayer(NewObject<ULocalPlayer>(GEngine));
	APlayerState* PS = World->SpawnActor<APlayerState>();
	PS->SetIsABot(false);
	PC->PlayerState = PS;
	Pawn->SetPlayerState(PS);
	Pawn->SetController(PC);
	PC->SetPawn(Pawn);
	PC->SetViewTarget(Pawn);
	Pawn->DispatchBeginPlay();
	PC->SetIgnoreLookInput(true);
	const bool bInputEnabledBefore = Pawn->InputEnabled();
	const TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
	const auto SpawnTrigger = [&](FName Id, ULevelSequence* Sequence)
	{
		const FTransform Transform(FVector(5000, 5000, 1000));
		AMVBossIntroTrigger* Trigger = World->SpawnActorDeferred<AMVBossIntroTrigger>(AMVBossIntroTrigger::StaticClass(), Transform,
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Trigger->Boss = Boss;
		Trigger->IntroId = Id;
		Trigger->IntroSequence = Sequence;
		Trigger->FinishSpawning(Transform);
		Trigger->DispatchBeginPlay();
		++GFrameCounter;
		World->GetTimerManager().Tick(0.05f);
		return Trigger;
	};
	ULevelSequence* Sequence = MaverickBossIntroTestsPrivate::MakeSequence(Boss, World);
	AMVBossIntroTrigger* Trigger = SpawnTrigger(TEXT("Boss_A"), Sequence);
	TestTrue(TEXT("Waiting encounter blocks AI debug/test activation"), AI->bBossIntroBlocked);
	TestFalse(TEXT("Boss overlap cannot start intro"), Trigger->TryStartIntro(Boss));
	Trigger->TriggerBox->OnComponentBeginOverlap.Broadcast(Trigger->TriggerBox, Pawn, Cast<UPrimitiveComponent>(Pawn->GetRootComponent()), 0, false, FHitResult());
	TestTrue(TEXT("Player block overlap starts sequence"), Trigger->IsIntroPlaying());
	TestTrue(TEXT("Gameplay actions blocked during intro"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	TestTrue(TEXT("Movement ignored during intro"), PC->IsMoveInputIgnored());
	TestFalse(TEXT("Repeated overlap cannot start twice"), Trigger->TryStartIntro(Pawn));
	AI->SetBossCombatEnabled(true, Pawn);
	TestFalse(TEXT("AI cannot bypass intro"), AI->BossPlayStart);
	ULevelSequencePlayer* Player = MaverickBossIntroTestsPrivate::FindPlayer(World);
	if (!TestNotNull(TEXT("Sequence player created"), Player)) return false;
	Player->Update(0.2f);
	Trigger->CancelIntro();
	FMVBossIntroSaveData Record;
	TestFalse(TEXT("Cancelled playback is not seen"), State->TryGetBossIntroCompleted(TEXT("Boss_A"), Record));
	TestFalse(TEXT("Cancellation releases action block"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	TestFalse(TEXT("Cancellation releases own move block"), PC->IsMoveInputIgnored());
	TestTrue(TEXT("Pre-existing look block preserved"), PC->IsLookInputIgnored());
	TestEqual(TEXT("Pawn input restored"), Pawn->InputEnabled(), bInputEnabledBefore);
	TestTrue(TEXT("Cancelled boss returns to initial location"), Boss->GetActorLocation().Equals(FVector(300, 200, 1000), 0.1));
	TestTrue(TEXT("Cancelled intro can retry"), Trigger->TryStartIntro(Pawn));
	Player = MaverickBossIntroTestsPrivate::FindPlayer(World);
	for (int32 Index = 0; Index < 20 && Trigger->IsIntroPlaying(); ++Index)
	{
		++GFrameCounter;
		World->TimeSeconds += 0.1;
		Player->Update(0.1f);
	}
	TestEqual(TEXT("Natural sequence end completes encounter"), Trigger->State, EMVBossIntroState::Completed);
	TestTrue(TEXT("Actual last transform persisted"), State->TryGetBossIntroCompleted(TEXT("Boss_A"), Record));
	TestTrue(TEXT("RestoreState cannot snap boss back"), Boss->GetActorLocation().Equals(FVector(800, 200, 1000), 1.0));
	TestTrue(TEXT("Record uses actual final capsule transform"), Record.BossTransform.Equals(Boss->GetActorTransform(), 0.01));
	TestTrue(TEXT("Successful completion enables combat"), AI->BossPlayStart && AI->TargetActor == Pawn && Boss->AttackTarget == Pawn);
	TestFalse(TEXT("Completion releases action block"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	TestTrue(TEXT("Camera target restored"), PC->GetViewTarget() == Pawn);
	TestFalse(TEXT("Completed intro never replays in same encounter"), Trigger->TryStartIntro(Pawn));
	TestTrue(TEXT("Completion writes save slot"), State->DoesSaveSlotExist(Slot, 0));
	Trigger->Destroy();
	State->ResetSaveData();
	TestTrue(TEXT("Slot reload restores seen record"), State->LoadFromSlot(Slot, 0));
	Boss->SetActorLocation(FVector(300, 200, 1000));
	AMVBossIntroTrigger* Revisit = SpawnTrigger(TEXT("Boss_A"), nullptr);
	TestEqual(TEXT("Seen encounter starts completed without sequence asset"), Revisit->State, EMVBossIntroState::Completed);
	TestTrue(TEXT("Revisit places boss at stored end location before overlap"), Boss->GetActorLocation().Equals(FVector(800, 200, 1000), 1.0));
	TestTrue(TEXT("Seen block entry goes directly to combat"), Revisit->TryStartIntro(Pawn));
	TestFalse(TEXT("Seen block entry does not lock actions"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	Revisit->Destroy();
	Boss->SetActorLocation(FVector(300, 200, 1000));
	AMVBossIntroTrigger* Interrupted = SpawnTrigger(TEXT("Boss_B"), Sequence);
	TestTrue(TEXT("Different encounter ID plays separately"), Interrupted->TryStartIntro(Pawn));
	Interrupted->Destroy();
	TestFalse(TEXT("Actor removal releases gameplay block"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	TestFalse(TEXT("Actor removal does not record completion"), State->TryGetBossIntroCompleted(TEXT("Boss_B"), Record));
	{
		UGameInstance* ReloadGI = NewObject<UGameInstance>(GEngine);
		ReloadGI->InitializeStandalone();
		UMVWorldStateSubsystem* ReloadState = ReloadGI->GetSubsystem<UMVWorldStateSubsystem>();
		TestTrue(TEXT("Fresh game instance loads selected save slot"), ReloadState->LoadFromSlot(Slot, 0));
		TestEqual(TEXT("Fresh game instance restores completed intros"), ReloadState->GetSaveData().BossIntros.Num(), 1);
		TestEqual(TEXT("Fresh game instance upgrades save version"), ReloadState->GetSaveData().SaveVersion, 3);
		UWorld* ReloadWorld = ReloadGI->GetWorld();
		ReloadGI->Shutdown();
		if (ReloadWorld) { ReloadWorld->DestroyWorld(false); GEngine->DestroyWorldContext(ReloadWorld); }
	}
	return !HasAnyErrors();
}
#endif
