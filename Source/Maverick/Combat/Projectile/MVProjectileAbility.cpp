#include "Combat/Projectile/MVProjectileAbility.h"

#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Combat/Projectile/MVProjectileTargeting.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "LockOnTargetComponent.h"

void UMVProjectileAbility::StartAbility_Implementation(int32 AbilityIndex)
{
    // 같은 Notify 구간에서 Start가 다시 와도 비용과 발사는 한 번만 처리한다.
    if (bAbilityActive)
    {
        return;
    }

    FMVProjectileLaunchContext Context;
    FTransform SpawnTransform;
    if (!TryBuildLaunchContext(AbilityIndex, Context, SpawnTransform))
    {
        return;
    }

    AMVCharacterBase* Character = GetOwnerCharacter();
    UMVCombatComponent* Combat = Cast<UMVCombatComponent>(OwnerComponent.Get());
    UWorld* World = IsValid(Character) ? Character->GetWorld() : nullptr;
    if (!IsValid(Combat) || !IsValid(World))
    {
        return;
    }

    // 기존 Ability의 비용 소모와 활성화 순서를 그대로 사용한다.
    Super::StartAbility_Implementation(AbilityIndex);
    if (!bAbilityActive || !IsValid(Character))
    {
        return;
    }

    AMVProjectileBase* Projectile =
        World->SpawnActorDeferred<AMVProjectileBase>(
            ProjectileClass.Get(),
            SpawnTransform,
            Character,
            Character,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (!IsValid(Projectile))
    {
        return;
    }

    if (!Projectile->InitializeProjectile(Context))
    {
        Projectile->FinishSpawning(SpawnTransform);
        Projectile->Destroy();
        return;
    }

    Projectile->FinishSpawning(SpawnTransform);

    if (!Combat->RegisterLaunchedProjectile(Projectile, this))
    {
        Projectile->Destroy();
        return;
    }

    if (!Projectile->ActivateProjectile())
    {
        // 등록 후 실패했으므로 EndPlay의 종료 통지로 등록이 해제된다.
        Projectile->Destroy();
    }
}

FMVProjectileSpawnInfo UMVProjectileAbility::ResolveSpawnInfo_Implementation(int32 AbilityIndex)
{
    FMVProjectileSpawnInfo Result;

    AMVCharacterBase* Character = GetOwnerCharacter();
    USkeletalMeshComponent* Mesh = IsValid(Character) ? Character->GetMesh() : nullptr;

    if (!IsValid(Mesh) ||
        LaunchSocket.IsNone() ||
        !Mesh->DoesSocketExist(LaunchSocket))
    {
        return Result;
    }

    Result.Location =
        Mesh->GetSocketLocation(LaunchSocket) +
        Character->GetActorRotation().RotateVector(LaunchOffset);
    Result.Direction = Character->GetActorForwardVector();

    Result.bValid = true;
    return Result;
}

bool UMVProjectileAbility::TryBuildLaunchContext(
    int32 AbilityIndex,
    FMVProjectileLaunchContext& OutContext,
    FTransform& OutSpawnTransform)
{
    AMVCharacterBase* Character = GetOwnerCharacter();

    if (!IsValid(Character) ||
        !IsValid(Character->GetWorld()) ||
        !ProjectileClass.Get() ||
        GetAttackInstanceId() == INDEX_NONE)
    {
        return false;
    }

    EMVProjectileTeam Team = EMVProjectileTeam::Unknown;
    if (Cast<AMVPlayerCharacter>(Character))
    {
        Team = EMVProjectileTeam::Player;
    }
    else if (Cast<AMVEnemy>(Character))
    {
        Team = EMVProjectileTeam::Enemy;
    }

    if (Team == EMVProjectileTeam::Unknown)
    {
        return false;
    }

    const FMVProjectileSpawnInfo SpawnInfo = ResolveSpawnInfo(AbilityIndex);
    const FVector& Direction = SpawnInfo.Direction;
    const FVector& Location = SpawnInfo.Location;

    const bool bValidDirection =
        FMath::IsFinite(Direction.X) &&
        FMath::IsFinite(Direction.Y) &&
        FMath::IsFinite(Direction.Z) &&
        !Direction.IsNearlyZero();

    const bool bValidLocation =
        FMath::IsFinite(Location.X) &&
        FMath::IsFinite(Location.Y) &&
        FMath::IsFinite(Location.Z);

    if (!SpawnInfo.bValid || !bValidDirection || !bValidLocation)
    {
        return false;
    }

    const UMVWeaponComponent* WeaponComponent = Character->FindComponentByClass<UMVWeaponComponent>();

    FMVWeaponHitSnapshot WeaponSnapshot;
    float PoiseDamage = 0.0f;

    if (IsValid(WeaponComponent))
    {
        WeaponSnapshot = WeaponComponent->CaptureWeaponHitSnapshot();

        if (WeaponSnapshot.bValid)
        {
            const FMVEquippedWeaponState WeaponState = WeaponComponent->GetEquippedWeaponState();

            PoiseDamage =
                FMath::Max(0.0f, WeaponState.WeaponPoise) *
                FMath::Max(0.0f, AbilityData.PoiseAmountMultiplier);
        }
    }

    // 무기 정보가 없을 때는 현재 HitResolver의 맨손 기본 공격력을 복사한다.
    if (!WeaponSnapshot.bValid)
    {
        const UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(Character);
        if (!IsValid(Resolver))
        {
            return false;
        }

        WeaponSnapshot = FMVWeaponHitSnapshot();
        WeaponSnapshot.AttackPower = FMath::Max(0.0f, Resolver->FallbackAttackPower);
        WeaponSnapshot.bValid = WeaponSnapshot.AttackPower > 0.0f;
    }

    if (!WeaponSnapshot.bValid)
    {
        return false;
    }

    OutContext = FMVProjectileLaunchContext();
    OutContext.Attacker = Character;
    OutContext.Team = Team;
    OutContext.AttackInstanceId = GetAttackInstanceId();
    OutContext.AttackTypes = ProjectileAttackType;
    OutContext.LaunchDirection = Direction.GetSafeNormal();

    const AMVProjectileBase* ProjectileDefaults = ProjectileClass.GetDefaultObject();

    if (IsValid(ProjectileDefaults) && ProjectileDefaults->IsHomingEnabled())
    {
        AActor* CurrentTarget = nullptr;

        if (const AMVEnemy* Enemy = Cast<AMVEnemy>(Character))
        {
            CurrentTarget = Enemy->AttackTarget.Get();
        }
        else if (TargetingSettings.Mode !=
                     EMVProjectileTargetSelectionMode::ProvidedOnly &&
                 TargetingSettings.bPreferCurrentTarget)
        {
            const ULockOnTargetComponent* LockOn = Character->FindComponentByClass<ULockOnTargetComponent>();

            if (IsValid(LockOn))
            {
                CurrentTarget = LockOn->GetTargetActor();
            }
        }

        OutContext.HomingTarget = MVProjectileTargeting::SelectTarget(
            Character,
            Team,
            SpawnInfo.HomingTarget.Get(),
            CurrentTarget,
            Location,
            ProjectileDefaults->GetTargetingCollision(),
            TargetingSettings);
    }

    OutContext.WeaponSnapshot = WeaponSnapshot;
    OutContext.DamageMultiplier = AbilityData.DamageMultiplier;
    OutContext.GroggyDamageMultiplier = AbilityData.GroggyDamageMultiplier;
    OutContext.PoiseDamage = PoiseDamage;
    OutContext.HitReactionType = ProjectileHitReaction;
    OutContext.HitLaunchData = GetHitLaunchData();

    OutSpawnTransform = FTransform(
        OutContext.LaunchDirection.Rotation(),
        Location);

    return true;
}