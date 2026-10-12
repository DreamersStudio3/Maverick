#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Character/MVCharacterBase.h"
#include "Combat/Duelist/MVDuelistResonanceComponent.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Misc/ScopeExit.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVDuelistResonanceTest,
	"Maverick.Combat.DuelistResonance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVDuelistResonanceTest::RunTest(const FString& Parameters)
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
	AMVCharacterBase* Player = World->SpawnActor<AMVCharacterBase>(FVector(500, 0, 1000), FRotator::ZeroRotator, Spawn);
	Player->DispatchBeginPlay();
	APlayerController* PC = World->SpawnActor<APlayerController>();
	APlayerState* PS = World->SpawnActor<APlayerState>();
	PC->PlayerState = PS;
	Player->SetPlayerState(PS);
	Player->SetController(PC);
	PC->SetPawn(Player);
	Player->StatComponent->SetMaxHP(1000);
	Player->StatComponent->SetCurrentHP(1000);
	Player->StatComponent->Defence = 99999;
	Player->WeaponComponent->BareHandWeapon.AttackPower = 10;
	Player->WeaponComponent->EquipBareHand();
	const auto SpawnBoss = [&]()
	{
		AMVCharacterBase* Boss = World->SpawnActor<AMVCharacterBase>(FVector(400, 0, 1000), FRotator::ZeroRotator, Spawn);
		Boss->DispatchBeginPlay();
		Boss->StatComponent->SetMaxHP(1000);
		Boss->StatComponent->SetCurrentHP(1000);
		Boss->StatComponent->SetMaxGroggy(100);
		Boss->StatComponent->SetInitialPoise(75);
		UMVDuelistResonanceComponent* Component = NewObject<UMVDuelistResonanceComponent>(Boss);
		Boss->AddInstanceComponent(Component);
		Component->RegisterComponent();
		Component->ArenaCenter = FVector(0, 0, 1000);
		return Component;
	};
	UMVDuelistResonanceComponent* Success = SpawnBoss();
	UMVStatComponent* Stat = Success->GetOwner()->FindComponentByClass<UMVStatComponent>();
	Stat->SetMaxHP(2000);
	TestFalse(TEXT("Max HP initialization cannot falsely reserve resonance"), Success->IsResonancePending());
	Stat->SetMaxHP(1000);
	Stat->SetCurrentHP(501);
	TestFalse(TEXT("Above 50 percent does not reserve"), Success->IsResonancePending());
	Stat->SetCurrentHP(500);
	TestTrue(TEXT("Exact 50 percent reserves next attack"), Success->IsResonancePending());
	TestFalse(TEXT("HP event does not start mid-attack"), Success->IsResonanceActive());
	const EMovementMode PreviousMovementMode = Cast<AMVCharacterBase>(Success->GetOwner())->GetCharacterMovement()->MovementMode;
	TestTrue(TEXT("Next task starts reserved mechanic"), Success->StartResonance());
	static_cast<UActorComponent*>(Success)->TickComponent(0.75f, LEVELTICK_All, nullptr);
	TestEqual(TEXT("Landing starts channel"), Success->Phase, EMVDuelistResonancePhase::Channel);
	TestEqual(TEXT("Full 5 second window after jump"), Success->ChannelRemaining, 5.0f);
	TestEqual(TEXT("Channel poise is exactly 40"), Stat->ConstantPoise, 40.0f);
	FMVHitResolveRequest Hit;
	Hit.Attacker = Player;
	Hit.Victim = Cast<AMVCharacterBase>(Success->GetOwner());
	Hit.DamageMultiplier = 0;
	Hit.GroggyDamageMultiplier = 100;
	Hit.PoiseDamage = 39;
	Hit.HitReactionType = EMVActionHitReactionType::Airborne;
	FMVResolvedHitData Resolved;
	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(World);
	TestTrue(TEXT("Normal hit passes through resolver"), Resolver->ResolveAttackHit(Hit, Resolved));
	TestEqual(TEXT("High groggy damage does not bypass required poise"), Stat->ConstantPoise, 1.0f);
	TestFalse(TEXT("39 poise cannot interrupt"), Stat->IsGroggy());
	TestEqual(TEXT("Channel ignores airborne until break"), Resolved.HitReactionType, EMVActionHitReactionType::None);
	Hit.PoiseDamage = 1;
	Resolver->ResolveAttackHit(Hit, Resolved);
	TestTrue(TEXT("40 total poise induces actual groggy"), Stat->IsGroggy());
	TestEqual(TEXT("Groggy interrupts wipe"), Success->Phase, EMVDuelistResonancePhase::Groggy);
	TestFalse(TEXT("Scoped poise window cleaned after interrupt"), Stat->IsPoiseBreakGroggyWindowActive());
	TestEqual(TEXT("Original poise restored"), Stat->ConstantPoise, 75.0f);
	TestEqual(TEXT("Successful interrupt never damages player"), Player->StatComponent->CurrentHP, 1000.0f);
	Stat->SetCurrentGroggy(0);
	static_cast<UActorComponent*>(Success)->TickComponent(0.1f, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Groggy completion releases Task"), Success->IsResonanceActive());
	TestEqual(TEXT("Original movement mode restored"), Cast<AMVCharacterBase>(Success->GetOwner())->GetCharacterMovement()->MovementMode.GetValue(), PreviousMovementMode);
	Stat->SetCurrentHP(900);
	Stat->SetCurrentHP(450);
	TestFalse(TEXT("Healing and threshold recross do not repeat"), Success->IsResonancePending());
	UMVDuelistResonanceComponent* Failure = SpawnBoss();
	UMVStatComponent* FailureStat = Failure->GetOwner()->FindComponentByClass<UMVStatComponent>();
	FailureStat->SetCurrentHP(500);
	Failure->StartResonance();
	static_cast<UActorComponent*>(Failure)->TickComponent(0.75f, LEVELTICK_All, nullptr);
	Hit.Victim = Cast<AMVCharacterBase>(Failure->GetOwner());
	Hit.PoiseDamage = 0;
	Hit.GroggyDamageMultiplier = 1;
	Resolver->ResolveAttackHit(Hit, Resolved);
	TestEqual(TEXT("Existing groggy-only attacks reduce channel poise"), FailureStat->ConstantPoise, 30.0f);
	World->TimeSeconds += 4.99f;
	static_cast<UActorComponent*>(Failure)->TickComponent(0.1f, LEVELTICK_All, nullptr);
	TestEqual(TEXT("No early explosion"), Player->StatComponent->CurrentHP, 1000.0f);
	World->TimeSeconds += 0.02f;
	static_cast<UActorComponent*>(Failure)->TickComponent(0.1f, LEVELTICK_All, nullptr);
	TestEqual(TEXT("Fixed 80 percent max HP ignores defence"), Player->StatComponent->CurrentHP, 200.0f);
	static_cast<UActorComponent*>(Failure)->TickComponent(1, LEVELTICK_All, nullptr);
	TestEqual(TEXT("Failure damage occurs once"), Player->StatComponent->CurrentHP, 200.0f);
	TestFalse(TEXT("Failure cleanup restores window"), FailureStat->IsPoiseBreakGroggyWindowActive());
	UMVDuelistResonanceComponent* Cancel = SpawnBoss();
	UMVStatComponent* CancelStat = Cancel->GetOwner()->FindComponentByClass<UMVStatComponent>();
	CancelStat->SetCurrentHP(500);
	Cancel->StartResonance();
	static_cast<UActorComponent*>(Cancel)->TickComponent(0.75f, LEVELTICK_All, nullptr);
	CancelStat->SetCurrentHP(0);
	TestFalse(TEXT("Boss death cancels channel"), Cancel->IsResonanceActive());
	TestFalse(TEXT("Boss death restores scoped poise"), CancelStat->IsPoiseBreakGroggyWindowActive());
	TestEqual(TEXT("Boss death never explodes"), Player->StatComponent->CurrentHP, 200.0f);
	return !HasAnyErrors();
}
#endif
