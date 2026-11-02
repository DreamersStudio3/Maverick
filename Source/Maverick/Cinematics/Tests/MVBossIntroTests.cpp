#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "AI/Controller/MVAIController.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Cinematics/Boss/MVBossIntroTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetTree.h"
#include "Components/MVInputManagerComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerInput.h"
#include "InputKeyEventArgs.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieScene.h"
#include "MovieSceneObjectBindingID.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "System/MVWorldStateSubsystem.h"
#include "UI/BossHUD/MVBossAttackTestWidget.h"
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
	PC->PlayerInput = NewObject<UPlayerInput>(PC);
	UMVBossAttackTestWidget* MouseWidget = nullptr;
	const auto BossIntroPressDebugKey = [&](const FKey Key)
	{
		PC->PlayerInput->FlushPressedKeys();
		PC->InputKey(FInputKeyEventArgs(nullptr, INPUTDEVICEID_NONE, Key, IE_Pressed, FPlatformTime::Cycles64()));
		TArray<UInputComponent*> InputStack;
		PC->PlayerInput->ProcessInputStack(InputStack, 0.016f, false);
		TestTrue(TEXT("Debug key reaches player input"), PC->WasInputKeyJustPressed(Key));
		AI->Tick(0.016f);
		if (MouseWidget) MouseWidget->NativeTick(FGeometry(), 0.016f);
		PC->PlayerInput->FlushPressedKeys();
	};
	Pawn->DispatchBeginPlay();
	PC->SetIgnoreLookInput(true);
	const bool bInputEnabledBefore = Pawn->InputEnabled();
	const TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
	MouseWidget = CreateWidget<UMVBossAttackTestWidget>(PC);
	if (!TestNotNull(TEXT("Mouse input test widget created"), MouseWidget)) return false;
	MouseWidget->TestBoss = Boss;
	MouseWidget->BaseAttackButton = NewObject<UButton>(MouseWidget);
	MouseWidget->QAttackButton = NewObject<UButton>(MouseWidget);
	MouseWidget->WAttackButton = NewObject<UButton>(MouseWidget);
	MouseWidget->PazeChangeAttackButton = NewObject<UButton>(MouseWidget);
	MouseWidget->WidgetTree->RootWidget = MouseWidget->WidgetTree->ConstructWidget<UOverlay>();
	MouseWidget->SetVisibility(ESlateVisibility::Visible);
	const TSharedRef<SWidget> MouseSlateWidget = MouseWidget->TakeWidget();
	TestTrue(TEXT("Mouse input fixture is visible and owns input"), MouseWidget->IsVisible() && MouseWidget->CanControlMouseInput());
	TestTrue(TEXT("Initial UI mode shows cursor"), MouseWidget->bUIMouseMode && PC->bShowMouseCursor);
	BossIntroPressDebugKey(EKeys::LeftMouseButton);
	TestFalse(TEXT("Viewport click enters game mode"), MouseWidget->bUIMouseMode);
	TestFalse(TEXT("Viewport click hides cursor"), PC->bShowMouseCursor);
	MouseWidget->NativeTick(FGeometry(), 0.016f);
	TestFalse(TEXT("Subsequent widget tick preserves game cursor mode"), PC->bShowMouseCursor);
	BossIntroPressDebugKey(EKeys::J);
	TestTrue(TEXT("J restores UI mouse mode and cursor"), MouseWidget->bUIMouseMode && PC->bShowMouseCursor);
	MouseWidget->NativeTick(FGeometry(), 0.016f);
	TestTrue(TEXT("Subsequent widget tick preserves UI cursor mode"), PC->bShowMouseCursor);
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
	const FTransform LegacyTransform(FVector(5000, 200, 1000));
	TestTrue(TEXT("Legacy completed record seeded"), State->RecordBossIntroCompleted(TEXT("Boss_A"), LegacyTransform));
	TestTrue(TEXT("Legacy save available before playback"), State->SaveToSlot(Slot, 0));
	AMVBossIntroTrigger* Trigger = SpawnTrigger(TEXT("Boss_A"), Sequence);
	TestEqual(TEXT("Legacy completed record does not skip intro"), Trigger->State, EMVBossIntroState::Waiting);
	TestTrue(TEXT("Legacy record does not move boss on initialization"), Boss->GetActorLocation().Equals(FVector(300, 200, 1000), 0.1));
	TestFalse(TEXT("Waiting encounter permits debug/test activation"), AI->bBossIntroBlocked);
	TestFalse(TEXT("Waiting encounter keeps automatic AI idle"), AI->BossPlayStart);
	BossIntroPressDebugKey(EKeys::K);
	TestTrue(TEXT("K starts AI before block entry and preserves patrol target"), AI->BossPlayStart && AI->TargetActor == nullptr && Boss->AttackTarget == nullptr);
	AI->SetBossCombatEnabled(false, nullptr);
	BossIntroPressDebugKey(EKeys::L);
	TestTrue(TEXT("L independently starts combat with player target before block entry"), AI->BossPlayStart && AI->TargetActor == Pawn && Boss->AttackTarget == Pawn);
	TestFalse(TEXT("Boss overlap cannot start intro"), Trigger->TryStartIntro(Boss));
	Trigger->TriggerBox->OnComponentBeginOverlap.Broadcast(Trigger->TriggerBox, Pawn, Cast<UPrimitiveComponent>(Pawn->GetRootComponent()), 0, false, FHitResult());
	TestTrue(TEXT("Player block overlap starts sequence"), Trigger->IsIntroPlaying());
	TestTrue(TEXT("Only actual playback blocks debug/test activation"), AI->bBossIntroBlocked);
	MouseWidget->SetUIMouseMode(false);
	TestTrue(TEXT("Cinematic rejects mouse mode override"), MouseWidget->bUIMouseMode);
	BossIntroPressDebugKey(EKeys::K);
	BossIntroPressDebugKey(EKeys::L);
	TestFalse(TEXT("K/L cannot start combat during playback"), AI->BossPlayStart);
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
	TestTrue(TEXT("Cancellation preserves legacy record"), State->TryGetBossIntroCompleted(TEXT("Boss_A"), Record));
	TestTrue(TEXT("Cancellation leaves saved placement unchanged"), Record.BossTransform.Equals(LegacyTransform));
	TestFalse(TEXT("Cancellation re-enables debug/test activation"), AI->bBossIntroBlocked);
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
	TestTrue(TEXT("Legacy record retained after completion"), State->TryGetBossIntroCompleted(TEXT("Boss_A"), Record));
	TestTrue(TEXT("RestoreState cannot snap boss back"), Boss->GetActorLocation().Equals(FVector(800, 200, 1000), 1.0));
	TestTrue(TEXT("Completion does not replace legacy saved placement"), Record.BossTransform.Equals(LegacyTransform));
	TestTrue(TEXT("Successful completion enables combat"), AI->BossPlayStart && AI->TargetActor == Pawn && Boss->AttackTarget == Pawn);
	TestFalse(TEXT("Completion releases action block"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	TestTrue(TEXT("Camera target restored"), PC->GetViewTarget() == Pawn);
	TestFalse(TEXT("Completed intro never replays in same encounter"), Trigger->TryStartIntro(Pawn));
	TestFalse(TEXT("Intro does not dirty saved progression"), State->IsSaveDataDirty());
	Trigger->Destroy();
	State->ResetSaveData();
	TestTrue(TEXT("Slot reload preserves legacy record"), State->LoadFromSlot(Slot, 0));
	Boss->SetActorLocation(FVector(300, 200, 1000));
	AMVBossIntroTrigger* Revisit = SpawnTrigger(TEXT("Boss_A"), nullptr);
	TestEqual(TEXT("Recreated encounter waits despite legacy completion"), Revisit->State, EMVBossIntroState::Waiting);
	TestTrue(TEXT("Recreated encounter preserves placed boss location"), Boss->GetActorLocation().Equals(FVector(300, 200, 1000), 1.0));
	TestFalse(TEXT("Waiting for entry permits debug/test activation"), AI->bBossIntroBlocked);
	TestFalse(TEXT("Missing sequence cannot be bypassed by legacy completion"), Revisit->TryStartIntro(Pawn));
	Revisit->IntroSequence = Sequence;
	TestTrue(TEXT("New encounter plays after save reload"), Revisit->TryStartIntro(Pawn));
	TestTrue(TEXT("Replayed intro locks actions"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	Revisit->CancelIntro();
	Revisit->Destroy();
	Boss->SetActorLocation(FVector(300, 200, 1000));
	AMVBossIntroTrigger* Interrupted = SpawnTrigger(NAME_None, Sequence);
	TestTrue(TEXT("Intro no longer requires a persistent record ID"), Interrupted->TryStartIntro(Pawn));
	Interrupted->Destroy();
	TestFalse(TEXT("Actor removal releases gameplay block"), Pawn->InputManagerComponent->IsGameplayInputBlocked());
	TestFalse(TEXT("Actor removal does not record completion"), State->TryGetBossIntroCompleted(TEXT("Boss_B"), Record));
	{
		UGameInstance* ReloadGI = NewObject<UGameInstance>(GEngine);
		ReloadGI->InitializeStandalone();
		UMVWorldStateSubsystem* ReloadState = ReloadGI->GetSubsystem<UMVWorldStateSubsystem>();
		TestTrue(TEXT("Fresh game instance loads selected save slot"), ReloadState->LoadFromSlot(Slot, 0));
		TestEqual(TEXT("Fresh game instance preserves legacy save compatibility"), ReloadState->GetSaveData().BossIntros.Num(), 1);
		TestEqual(TEXT("Fresh game instance upgrades save version"), ReloadState->GetSaveData().SaveVersion, 3);
		UWorld* ReloadWorld = ReloadGI->GetWorld();
		ReloadGI->Shutdown();
		if (ReloadWorld) { ReloadWorld->DestroyWorld(false); GEngine->DestroyWorldContext(ReloadWorld); }
	}
	return !HasAnyErrors();
}
#endif
