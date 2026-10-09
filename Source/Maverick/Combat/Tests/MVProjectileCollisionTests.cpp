#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMVProjectileCollisionTest,
    "Maverick.Combat.Projectile.InitialOverlapAndTeamFilter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVProjectileCollisionTest::RunTest(const FString& Parameters)
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

    const FTransform PlayerTransform(FRotator::ZeroRotator, FVector::ZeroVector);
    AMVPlayerCharacter* Player = World->SpawnActorDeferred<AMVPlayerCharacter>(
        AMVPlayerCharacter::StaticClass(), PlayerTransform,
        nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (Player)
    {
        if (UMVCombatComponent* Combat = Player->CombatComponent.Get())
        {
            Combat->AttackChooserTable = FSoftObjectPath();
            Combat->FallbackAttackActionTable = FSoftObjectPath();
        }
        Player->FinishSpawning(PlayerTransform);
    }

    AMVEnemy* Enemy = World->SpawnActor<AMVEnemy>(
        FVector(200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Spawn);
    AMVEnemy* Ally = World->SpawnActor<AMVEnemy>(
        FVector(600.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Spawn);

    const bool bActorsReady =
        TestNotNull(TEXT("Player"), Player) &&
        TestNotNull(TEXT("Enemy"), Enemy) &&
        TestNotNull(TEXT("Ally"), Ally);

    if (bActorsReady)
    {
        Player->DispatchBeginPlay();
        Enemy->DispatchBeginPlay();
        Ally->DispatchBeginPlay();

        const bool bStatsReady =
            TestNotNull(TEXT("Player stats"), Player->StatComponent.Get()) &&
            TestNotNull(TEXT("Enemy stats"), Enemy->StatComponent.Get()) &&
            TestNotNull(TEXT("Ally stats"), Ally->StatComponent.Get());

        if (bStatsReady)
        {
            Player->StatComponent->SetMaxHP(100.0f);
            Player->StatComponent->SetCurrentHP(100.0f);
            Player->StatComponent->CriticalPercent = 0.0f;

            Enemy->StatComponent->SetMaxHP(100.0f);
            Enemy->StatComponent->SetCurrentHP(100.0f);
            Enemy->StatComponent->SetSkillDefence(0.0f);

            Ally->StatComponent->SetMaxHP(100.0f);
            Ally->StatComponent->SetCurrentHP(100.0f);

            const auto SpawnShot =
                [World](AMVCharacterBase* Shooter,
                        EMVProjectileTeam Team,
                        const FVector& Location,
                        int32 AttackId) -> AMVProjectileBase*
            {
                FMVProjectileLaunchContext Context;
                Context.Attacker = Shooter;
                Context.Team = Team;
                Context.AttackInstanceId = AttackId;
                Context.AttackTypes = EMVAttackTypes::SkillAttack;
                Context.LaunchDirection = FVector::ForwardVector;
                Context.WeaponSnapshot.AttackPower = 20.0f;
                Context.WeaponSnapshot.bValid = true;

                const FTransform Transform(FRotator::ZeroRotator, Location);
                AMVProjectileBase* Projectile =
                    World->SpawnActorDeferred<AMVProjectileBase>(
                        AMVProjectileBase::StaticClass(),
                        Transform,
                        Shooter,
                        Shooter,
                        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

                if (!Projectile)
                {
                    return nullptr;
                }

                if (!Projectile->InitializeProjectile(Context))
                {
                    Projectile->FinishSpawning(Transform);
                    Projectile->Destroy();
                    return nullptr;
                }

                Projectile->FinishSpawning(Transform);
                return Projectile;
            };

            AMVProjectileBase* PlayerShot = SpawnShot(
                Player, EMVProjectileTeam::Player,
                Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f),
                101);

            if (TestNotNull(TEXT("Player projectile"), PlayerShot))
            {
                const bool bActivated = PlayerShot->ActivateProjectile();
                TestTrue(TEXT("Player projectile activates"), bActivated);
                TestTrue(TEXT("Initial overlap damages enemy once"),
                    FMath::IsNearlyEqual(
                        Enemy->StatComponent->CurrentHP, 80.0f));
            }

            AMVProjectileBase* EnemyShot = SpawnShot(
                Enemy, EMVProjectileTeam::Enemy,
                Ally->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f),
                102);

            if (TestNotNull(TEXT("Enemy projectile"), EnemyShot))
            {
                TestTrue(TEXT("Enemy projectile activates"),
                    EnemyShot->ActivateProjectile());
                TestEqual(TEXT("Enemy projectile ignores ally"),
                    Ally->StatComponent->CurrentHP, 100.0f);
            }

            AMVProjectileBase* OwnShot = SpawnShot(
                Player, EMVProjectileTeam::Player,
                Player->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f),
                103);

            if (TestNotNull(TEXT("Owner-overlapping projectile"), OwnShot))
            {
                TestTrue(TEXT("Owner-overlapping projectile activates"),
                    OwnShot->ActivateProjectile());
                TestEqual(TEXT("Projectile ignores its shooter"),
                    Player->StatComponent->CurrentHP, 100.0f);
            }
        }
    }

    World->EndPlay(EEndPlayReason::Quit);
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return !HasAnyErrors();
}

#endif
