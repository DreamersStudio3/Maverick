#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Combat/Projectile/MVProjectileTargeting.h"
#include "Components/BoxComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMVProjectileTargetSelectionTest,
    "Maverick.Combat.Projectile.AutoTargetSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVProjectileTargetSelectionTest::RunTest(const FString& Parameters)
{
    const UWorld::InitializationValues Values = UWorld::InitializationValues()
        .AllowAudioPlayback(false)
        .CreatePhysicsScene(true)
        .CreateNavigation(false)
        .CreateAISystem(false)
        .ShouldSimulatePhysics(false);

    UWorld* World = UWorld::CreateWorld(
        EWorldType::Game, false, NAME_None, nullptr, true,
        ERHIFeatureLevel::Num, &Values);

    if (!TestNotNull(TEXT("Test world"), World))
    {
        return false;
    }

    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());
    World->BeginPlay();

    FActorSpawnParameters Spawn;
    Spawn.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const FTransform PlayerTransform(
        FRotator::ZeroRotator, FVector::ZeroVector);

    AMVPlayerCharacter* Player =
        World->SpawnActorDeferred<AMVPlayerCharacter>(
            AMVPlayerCharacter::StaticClass(), PlayerTransform,
            nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (Player)
    {
        if (UMVCombatComponent* Combat = Player->CombatComponent.Get())
        {
            Combat->AttackChooserTable = FSoftObjectPath();
            Combat->FallbackAttackActionTable = FSoftObjectPath();
        }
        Player->FinishSpawning(PlayerTransform);
    }

    AMVEnemy* NearFront = World->SpawnActor<AMVEnemy>(
        FVector(350.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Spawn);
    AMVEnemy* FarFront = World->SpawnActor<AMVEnemy>(
    FVector(700.0f, 400.0f, 0.0f), FRotator::ZeroRotator, Spawn);
    AMVEnemy* Behind = World->SpawnActor<AMVEnemy>(
        FVector(-200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Spawn);

    AMVProjectileBase* Probe = World->SpawnActor<AMVProjectileBase>(
        FVector(0.0f, 0.0f, 300.0f),
        FRotator::ZeroRotator, Spawn);

    const bool bActorsReady =
        TestNotNull(TEXT("Player"), Player) &&
        TestNotNull(TEXT("Near-front enemy"), NearFront) &&
        TestNotNull(TEXT("Far-front enemy"), FarFront) &&
        TestNotNull(TEXT("Behind enemy"), Behind) &&
        TestNotNull(TEXT("Projectile collision source"), Probe);

    if (bActorsReady)
    {
        Player->DispatchBeginPlay();
        NearFront->DispatchBeginPlay();
        FarFront->DispatchBeginPlay();
        Behind->DispatchBeginPlay();

        const bool bStatsReady =
            TestNotNull(TEXT("Near-front stats"), NearFront->StatComponent.Get()) &&
            TestNotNull(TEXT("Far-front stats"), FarFront->StatComponent.Get()) &&
            TestNotNull(TEXT("Behind stats"), Behind->StatComponent.Get());

        if (bStatsReady)
        {
            for (AMVEnemy* Enemy : {NearFront, FarFront, Behind})
            {
                Enemy->StatComponent->SetMaxHP(100.0f);
                Enemy->StatComponent->SetCurrentHP(100.0f);
            }

            FMVProjectileTargetingSettings Settings;
            Settings.SearchRadius = 1000.0f;
            Settings.FrontHalfAngle = 45.0f;
            Settings.bPreferCurrentTarget = false;
            Settings.bRequireClearPath = false;

            const FVector LaunchLocation(0.0f, 0.0f, 100.0f);
            const USphereComponent* Collision =
                Probe->GetTargetingCollision();

            Settings.Mode = EMVProjectileTargetSelectionMode::Nearest;
            TestTrue(TEXT("Nearest includes the enemy behind"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    nullptr, nullptr, LaunchLocation,
                    Collision, Settings) == Behind);

            Settings.Mode = EMVProjectileTargetSelectionMode::NearestInFront;
            TestTrue(TEXT("Nearest in front chooses the closer front enemy"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    nullptr, nullptr, LaunchLocation,
                    Collision, Settings) == NearFront);

            Settings.bPreferCurrentTarget = true;
            TestTrue(TEXT("Current target takes priority when enabled"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    nullptr, FarFront, LaunchLocation,
                    Collision, Settings) == FarFront);

                        TestTrue(TEXT("Explicit target takes priority"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    Behind, FarFront, LaunchLocation,
                    Collision, Settings) == Behind);

            Settings.bPreferCurrentTarget = false;
            Settings.bRequireClearPath = true;

            AActor* Wall = World->SpawnActor<AActor>(
                FVector(175.0f, 0.0f, 50.0f),
                FRotator::ZeroRotator, Spawn);

            if (TestNotNull(TEXT("Blocking wall"), Wall))
            {
                UBoxComponent* Blocker = NewObject<UBoxComponent>(Wall);
                Wall->AddInstanceComponent(Blocker);
                Wall->SetRootComponent(Blocker);
                Blocker->SetBoxExtent(FVector(20.0f, 40.0f, 100.0f));
                Blocker->SetCollisionObjectType(ECC_WorldStatic);
                Blocker->SetCollisionResponseToAllChannels(ECR_Block);
                Blocker->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                Blocker->RegisterComponent();
                Blocker->SetWorldLocation(FVector(175.0f, 0.0f, 50.0f));

                TestTrue(TEXT("Blocked near enemy falls back to clear enemy"),
                    MVProjectileTargeting::SelectTarget(
                        Player, EMVProjectileTeam::Player,
                        nullptr, nullptr, LaunchLocation,
                        Collision, Settings) == FarFront);
            }

            Settings.bRequireClearPath = false;
            TestTrue(TEXT("Disabling path check permits near enemy"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    nullptr, nullptr, LaunchLocation,
                    Collision, Settings) == NearFront);

            NearFront->BeginInvincibility();
            TestTrue(TEXT("Invincible enemy is skipped"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    nullptr, nullptr, LaunchLocation,
                    Collision, Settings) == FarFront);
            NearFront->EndInvincibility();

            NearFront->StatComponent->OnDeathStarted.Clear();
            NearFront->StatComponent->SetCurrentHP(0.0f);
            TestTrue(TEXT("Enemy is marked dead"),
                NearFront->StatComponent->IsDead());
            TestTrue(TEXT("Dead enemy is skipped"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    nullptr, nullptr, LaunchLocation,
                    Collision, Settings) == FarFront);

            Settings.SearchRadius = 100.0f;
            TestNull(TEXT("No target inside the search radius"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    nullptr, nullptr, LaunchLocation,
                    Collision, Settings));

            Settings.Mode = EMVProjectileTargetSelectionMode::ProvidedOnly;
            TestTrue(TEXT("ProvidedOnly keeps an explicit target"),
                MVProjectileTargeting::SelectTarget(
                    Player, EMVProjectileTeam::Player,
                    Behind, nullptr, LaunchLocation,
                    Collision, Settings) == Behind);
        }
    }

    World->EndPlay(EEndPlayReason::Quit);
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return !HasAnyErrors();
}

#endif