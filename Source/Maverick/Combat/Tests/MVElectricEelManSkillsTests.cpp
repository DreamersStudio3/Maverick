#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Animation/NotifyStates/MVAnimNotifyState_Ability.h"
#include "AI/Controller/MVAIController.h"
#include "Character/MVCharacterBase.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManQAbility.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManWAbility.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManEAbility.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManRAbility.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManSkillComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Misc/ScopeExit.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVElectricEelManSkillsTest, "Maverick.Combat.ElectricEelMan.Skills",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVElectricEelManSkillsTest::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues Values = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	ON_SCOPE_EXIT { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); };
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform BossTransform(FRotator::ZeroRotator, FVector(0, 0, 1000));
	auto* Boss = World->SpawnActorDeferred<AMVCharacterBase>(AMVCharacterBase::StaticClass(), BossTransform,
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Boss->WeaponComponent->BareHandWeapon.AttackPower = 100.0f;
	Boss->FinishSpawning(BossTransform);
	auto* Player = World->SpawnActor<AMVCharacterBase>(FVector(200, 0, 1000), FRotator::ZeroRotator, Spawn);
	Boss->DispatchBeginPlay();
	Player->DispatchBeginPlay();
	TestEqual(TEXT("Test weapon initialized before ability execution"), Boss->WeaponComponent->GetEquippedWeaponAttackPower(), 100.0f);
	auto* PC = World->SpawnActor<APlayerController>();
	auto* PS = World->SpawnActor<APlayerState>();
	PS->SetIsABot(false);
	PC->PlayerState = PS;
	Player->SetPlayerState(PS);
	Player->SetController(PC);
	PC->SetPawn(Player);
	auto* AI = World->SpawnActor<AMVAIController>();
	Boss->SetController(AI);
	AI->SetPawn(Boss);
	AI->TargetActor = Player;
	Boss->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	Player->StatComponent->SetMaxHP(10000);
	Player->StatComponent->SetCurrentHP(10000);
	Player->StatComponent->SetNormalDefence(0);
	Boss->StatComponent->CriticalPercent = 0;
	FMVSkillDataTableColumn Row;
	Row.DamageMultiplier = 1;
	Row.GroggyDamageMultiplier = 0;
	auto* Notify = NewObject<UMVAnimNotifyState_Ability>();
	const FAnimNotifyEventReference Reference;
	const TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
	const auto Advance = [&](float Seconds)
	{
		for (int32 Index = 0; Index < FMath::CeilToInt(Seconds / 0.05f); ++Index)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, 0.05f);
		}
	};
	const auto Prepare = [&](UMVAbilityBase* Ability)
	{
		Ability->SetOwner(Boss->CombatComponent);
		Ability->InitAbility(Row);
		Ability->SetAttackInstanceId(100);
		Boss->CombatComponent->CurrentAbilityInstance = Ability;
	};
	auto* Q = NewObject<UMVElectricEelManQAbility>(Boss->CombatComponent);
	Q->bDrawDebug = false;
	Prepare(Q);
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1, Reference);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	auto* State = Boss->FindComponentByClass<UMVElectricEelManSkillComponent>();
	if (!TestNotNull(TEXT("Q creates persistent counter"), State)) return false;
	TestEqual(TEXT("Q records first activation"), State->SkillUseCount, 1);
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1, Reference);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	TestEqual(TEXT("Second notify window does not recount Q"), State->SkillUseCount, 1);
	auto* R = NewObject<UMVElectricEelManRAbility>(Boss->CombatComponent);
	Prepare(R);
	TestFalse(TEXT("R gated before nine uses"), R->CanActivateAbility());
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 6, Reference);
	TestFalse(TEXT("Rejected R remains inactive"), R->bAbilityActive);
	TestFalse(TEXT("Rejected R pays no cost"), R->bAbilityCostConsumed);

	auto* W = NewObject<UMVElectricEelManWAbility>(Boss->CombatComponent);
	W->JumpDistance = 0;
	W->JumpHeight = 0;
	W->bDrawDebug = false;
	Prepare(W);
	float HP = Player->StatComponent->CurrentHP;
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1, Reference);
	Advance(0.7f);
	TestEqual(TEXT("Fist takes priority over shockwave, full damage once"), Player->StatComponent->CurrentHP, HP - 100);
	TestEqual(TEXT("W counts once"), State->SkillUseCount, 2);
	Player->SetActorLocation(FVector(450, 0, 1000));
	Prepare(W);
	HP = Player->StatComponent->CurrentHP;
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 1, Reference);
	Advance(0.7f);
	TestEqual(TEXT("Shockwave only deals half damage"), Player->StatComponent->CurrentHP, HP - 50);

	auto* E = NewObject<UMVElectricEelManEAbility>(Boss->CombatComponent);
	E->bDrawDebug = false;
	Player->SetActorLocation(FVector(140, 0, 1000));
	Prepare(E);
	HP = Player->StatComponent->CurrentHP;
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 3, Reference);
	Advance(0.2f);
	TestTrue(TEXT("Grab blocks gameplay input"), Player->InputManagerComponent->IsGameplayInputBlocked());
	TestEqual(TEXT("Grab waits before electric damage"), Player->StatComponent->CurrentHP, HP);
	Advance(0.7f);
	TestEqual(TEXT("Grab delivers electric damage once"), Player->StatComponent->CurrentHP, HP - 100);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	TestFalse(TEXT("Notify cancellation releases captured player"), Player->InputManagerComponent->IsGameplayInputBlocked());
	TestEqual(TEXT("Grab restores original movement mode"), Player->GetCharacterMovement()->MovementMode.GetValue(), MOVE_Flying);
	Advance(1.0f);
	TestEqual(TEXT("No delayed damage after release"), Player->StatComponent->CurrentHP, HP - 100);
	Boss->SetActorLocation(FVector(0, 0, 1000));
	Player->SetActorLocation(FVector(500, 0, 1000));
	for (int32 Index = State->SkillUseCount; Index < 9; ++Index) State->RecordSkillUse();
	Prepare(R);
	R->bDrawDebug = false;
	TestTrue(TEXT("Nine uses unlock R"), R->CanActivateAbility());
	HP = Player->StatComponent->CurrentHP;
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 6, Reference);
	TestEqual(TEXT("R consumes counter on activation"), State->SkillUseCount, 0);
	Advance(2.8f);
	TestEqual(TEXT("R has no damage before charge completes"), Player->StatComponent->CurrentHP, HP);
	Advance(0.4f);
	TestEqual(TEXT("R first pulse at three seconds"), Player->StatComponent->CurrentHP, HP - 100);
	Advance(1.0f);
	TestEqual(TEXT("R second pulse at four seconds"), Player->StatComponent->CurrentHP, HP - 200);
	Advance(1.0f);
	TestEqual(TEXT("R third pulse at five seconds"), Player->StatComponent->CurrentHP, HP - 300);
	TestFalse(TEXT("R completes after three pulses"), R->bAbilityActive);
	Advance(1.0f);
	TestEqual(TEXT("R never delivers a fourth pulse"), Player->StatComponent->CurrentHP, HP - 300);
	for (int32 Index = 0; Index < 9; ++Index) State->RecordSkillUse();
	Prepare(R);
	Notify->NotifyBegin(Boss->GetMesh(), nullptr, 6, Reference);
	Advance(0.2f);
	Notify->NotifyEnd(Boss->GetMesh(), nullptr, Reference);
	HP = Player->StatComponent->CurrentHP;
	Advance(6.0f);
	TestEqual(TEXT("Cancelled R removes all pending pulses"), Player->StatComponent->CurrentHP, HP);
	TestEqual(TEXT("Cancelled R keeps consumed count"), State->SkillUseCount, 0);
	return !HasAnyErrors();
}
#endif
