#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Animation/NotifyStates/MVAnimNotifyState_Ability.h"
#include "Character/MVCharacterBase.h"
#include "Combat/MVBoxDamageAbility.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Misc/ScopeExit.h"
#include "TimerManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVBoxDamageAbilityTest,
	"Maverick.Combat.BoxDamageAbility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVBoxDamageAbilityTest::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues Values = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMVCharacterBase* Boss = World->SpawnActor<AMVCharacterBase>(FVector(0, 0, 1000), FRotator::ZeroRotator, Spawn);
	AMVCharacterBase* Player = World->SpawnActor<AMVCharacterBase>(FVector(350, 0, 1000), FRotator::ZeroRotator, Spawn);
	AMVCharacterBase* NonPlayer = World->SpawnActor<AMVCharacterBase>(FVector(150, 0, 1000), FRotator::ZeroRotator, Spawn);
	Boss->DispatchBeginPlay();
	Player->DispatchBeginPlay();
	NonPlayer->DispatchBeginPlay();
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	APlayerState* PlayerState = World->SpawnActor<APlayerState>();
	PlayerState->SetIsABot(false);
	Controller->PlayerState = PlayerState;
	Player->SetPlayerState(PlayerState);
	Player->SetController(Controller);
	Controller->SetPawn(Player);
	TestTrue(TEXT("Controlled test player"), Player->IsPlayerControlled());
	Player->StatComponent->SetMaxHP(1000.0f);
	Player->StatComponent->SetCurrentHP(1000.0f);
	Player->StatComponent->Defence = 5.0f;
	Player->StatComponent->SetCurrentGroggy(25.0f);
	NonPlayer->StatComponent->SetCurrentHP(100.0f);
	Boss->WeaponComponent->BareHandWeapon.AttackPower = 10.0f;
	Boss->WeaponComponent->EquipBareHand();
	TestEqual(TEXT("Controlled weapon power"), Boss->WeaponComponent->GetEquippedWeaponAttackPower(), 10.0f);
	UMVBoxDamageAbility* Ability = NewObject<UMVBoxDamageAbility>(Boss->CombatComponent);
	Boss->CombatComponent->CurrentAbilityInstance = Ability;
	FMVSkillDataTableColumn Data;
	Data.DamageMultiplier = 2.0f;
	Data.GroggyDamageMultiplier = 10.0f;
	Ability->SetOwner(Boss->CombatComponent);
	Ability->InitAbility(Data);
	Ability->SetAttackInstanceId(100);
	Ability->BoxSize = FVector(200, 100, 100);
	Ability->BoxOffset = FVector(150, 0, 0);
	Ability->bDrawDebug = false;
	Ability->HitLaunchData.LaunchDistance = 500.0f;
	Ability->HitLaunchData.LaunchDuration = 1.0f;
	Ability->HitLaunchData.LaunchVerticalSpeed = 1000.0f;
	UMVAnimNotifyState_Ability* Notify = NewObject<UMVAnimNotifyState_Ability>();
	Notify->AbilityClass = UMVBoxDamageAbility::StaticClass();
	const FAnimNotifyEventReference Reference;
	const TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
	const auto AdvanceBoxTestTime = [&]()
	{
		++GFrameCounter;
		World->GetTimerManager().Tick(0.05f);
	};
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1.0f, Reference);
	TestTrue(TEXT("Notify Begin activates box"), Ability->bAbilityActive);
	TestEqual(TEXT("BoxSize uses full lengths: outside player not damaged"), Player->StatComponent->CurrentHP, 1000.0f);
	TestEqual(TEXT("Non-player inside box excluded"), NonPlayer->StatComponent->CurrentHP, 100.0f);
	// 첫 TimerManager 프레임의 신규 타이머 등록 완료 후 범위 진입 검사
	AdvanceBoxTestTime();
	Player->SetActorLocation(FVector(150, 0, 1000));
	AdvanceBoxTestTime();
	TestEqual(TEXT("Player entering active box receives weapon damage minus defence"), Player->StatComponent->CurrentHP, 985.0f);
	for (int32 Index = 0; Index < 3; ++Index) AdvanceBoxTestTime();
	TestEqual(TEXT("Repeated overlaps apply damage only once per window"), Player->StatComponent->CurrentHP, 985.0f);
	TestEqual(TEXT("HP-only damage preserves groggy"), Player->StatComponent->CurrentGroggy, 25.0f);
	TestFalse(TEXT("No hit reaction action"), Player->ActionComponent->IsActionRunning());
	TestTrue(TEXT("Inherited launch defaults do not move player"), Player->GetCharacterMovement()->Velocity.IsNearlyZero());
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	AdvanceBoxTestTime();
	TestFalse(TEXT("Notify End deactivates box"), Ability->bAbilityActive);
	TestEqual(TEXT("No damage after Notify End"), Player->StatComponent->CurrentHP, 985.0f);
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1.0f, Reference);
	TestEqual(TEXT("Next activation allows one new hit"), Player->StatComponent->CurrentHP, 970.0f);
	IMVAbilityInterface::Execute_EndAbility(Ability);
	AdvanceBoxTestTime();
	TestFalse(TEXT("Explicit termination deactivates box"), Ability->bAbilityActive);
	TestEqual(TEXT("No damage after explicit termination"), Player->StatComponent->CurrentHP, 970.0f);
	Boss->CombatComponent->CurrentAbilityInstance = Ability;
	Ability->PrepareAbilityExecution();
	Boss->SetActorRotation(FRotator(0, 90, 0));
	Ability->BoxSize = FVector(300, 60, 100);
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1.0f, Reference);
	TestEqual(TEXT("Rotated box excludes old forward position"), Player->StatComponent->CurrentHP, 970.0f);
	Player->SetActorLocation(FVector(0, 150, 1000));
	AdvanceBoxTestTime();
	TestEqual(TEXT("Box offset and extents rotate with owner"), Player->StatComponent->CurrentHP, 955.0f);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	Player->BeginInvincibility();
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1.0f, Reference);
	AdvanceBoxTestTime();
	TestEqual(TEXT("Invincible player rejected"), Player->StatComponent->CurrentHP, 955.0f);
	Player->EndInvincibility();
	AdvanceBoxTestTime();
	TestEqual(TEXT("Eligible player can be hit later in same window"), Player->StatComponent->CurrentHP, 940.0f);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	Ability->BoxOffset = FVector(150, 0, 500);
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1.0f, Reference);
	TestEqual(TEXT("Vertical offset excludes player below box"), Player->StatComponent->CurrentHP, 940.0f);
	Player->SetActorLocation(FVector(0, 150, 1500));
	AdvanceBoxTestTime();
	TestEqual(TEXT("Vertical offset includes matching height"), Player->StatComponent->CurrentHP, 925.0f);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	Data.MpCost = 9999.0f;
	Ability->InitAbility(Data);
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1.0f, Reference);
	AdvanceBoxTestTime();
	TestFalse(TEXT("Failed ability cost leaves box inactive"), Ability->bAbilityActive);
	TestEqual(TEXT("Failed activation applies no damage"), Player->StatComponent->CurrentHP, 925.0f);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	return !HasAnyErrors();
}
#endif
