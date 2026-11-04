#include "Combat/Projectile/MVProjectileTargeting.h"

#include "Algo/Sort.h"
#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "CollisionQueryParams.h"
#include "Components/MVStatComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

bool MVProjectileTargeting::IsLivingHostile(
    const AMVCharacterBase* Shooter,
    EMVProjectileTeam Team,
    const AMVCharacterBase* Target)
{
    if (!IsValid(Shooter) ||
        !IsValid(Target) ||
        Shooter == Target ||
        !Shooter->GetWorld() ||
        Shooter->GetWorld() != Target->GetWorld() ||
        !IsValid(Target->GetRootComponent()))
    {
        return false;
    }

    const UMVStatComponent* Stats = Target->FindComponentByClass<UMVStatComponent>();

    if (!IsValid(Stats) || Stats->IsDead())
    {
        return false;
    }

    if (Team == EMVProjectileTeam::Player)
    {
        return Shooter->IsA<AMVPlayerCharacter>() &&
            Target->IsA<AMVEnemy>();
    }

    if (Team == EMVProjectileTeam::Enemy)
    {
        return Shooter->IsA<AMVEnemy>() &&
            Target->IsA<AMVPlayerCharacter>();
    }

    return false;
}

bool MVProjectileTargeting::CanAcquire(
    const AMVCharacterBase* Shooter,
    EMVProjectileTeam Team,
    const AMVCharacterBase* Target)
{
    return IsLivingHostile(Shooter, Team, Target) &&
        !Target->IsInvincible();
}

AActor* MVProjectileTargeting::SelectTarget(
    AMVCharacterBase* Shooter,
    EMVProjectileTeam Team,
    AActor* ExplicitTarget,
    AActor* CurrentTarget,
    const FVector& LaunchLocation,
    const USphereComponent* ProjectileCollision,
    const FMVProjectileTargetingSettings& Settings)
{
    if (!IsValid(Shooter) || !IsValid(Shooter->GetWorld()))
    {
        return nullptr;
    }

    UWorld* World = Shooter->GetWorld();
    const bool bAutomatic = Settings.Mode != EMVProjectileTargetSelectionMode::ProvidedOnly;

    if (bAutomatic &&
        (!IsValid(ProjectileCollision) ||
         !FMath::IsFinite(Settings.SearchRadius) ||
         Settings.SearchRadius <= 0.0f))
    {
        return nullptr;
    }

    const FVector SearchOrigin = Shooter->GetActorLocation();
    const double RadiusSquared = static_cast<double>(Settings.SearchRadius) * Settings.SearchRadius;

    const auto CanUseTarget =
        [&](AActor* Actor) -> bool
    {
        AMVCharacterBase* Candidate = Cast<AMVCharacterBase>(Actor);

        if (!CanAcquire(Shooter, Team, Candidate))
        {
            return false;
        }

        if (!bAutomatic)
        {
            return true;
        }

        if (FVector::DistSquared(SearchOrigin, Candidate->GetActorLocation()) > RadiusSquared)
        {
            return false;
        }

        if (!Settings.bRequireClearPath)
        {
            return true;
        }

        FCollisionQueryParams QueryParams(
            SCENE_QUERY_STAT(MVProjectileTargetPath), false, Shooter);
        const FCollisionResponseParams ResponseParams(
            ProjectileCollision->GetCollisionResponseToChannels());

        FHitResult BlockingHit;
        return !World->SweepSingleByChannel(
            BlockingHit,
            LaunchLocation,
            Candidate->GetRootComponent()->GetComponentLocation(),
            FQuat::Identity,
            ProjectileCollision->GetCollisionObjectType(),
            FCollisionShape::MakeSphere(
                ProjectileCollision->GetScaledSphereRadius()),
            QueryParams,
            ResponseParams);
    };

    if (CanUseTarget(ExplicitTarget))
    {
        return ExplicitTarget;
    }

    if ((!bAutomatic || Settings.bPreferCurrentTarget) &&
        CanUseTarget(CurrentTarget))
    {
        return CurrentTarget;
    }

    if (!bAutomatic)
    {
        return nullptr;
    }

    const FVector Forward3D = Shooter->GetActorForwardVector();
    const FVector2D Forward = FVector2D(Forward3D.X, Forward3D.Y).GetSafeNormal();

    const auto FacingDot =
        [&](const AMVCharacterBase* Candidate) -> double
    {
        const FVector Offset = Candidate->GetActorLocation() - SearchOrigin;
        const FVector2D Horizontal(Offset.X, Offset.Y);

        return Horizontal.IsNearlyZero()
            ? 1.0
            : FVector2D::DotProduct(
                  Forward, Horizontal.GetSafeNormal());
    };

    const double MinimumFrontDot = FMath::Cos(
        FMath::DegreesToRadians(
            FMath::Clamp(Settings.FrontHalfAngle, 0.0f, 180.0f)));

    TArray<FOverlapResult> Overlaps;
    const FCollisionQueryParams QueryParams(
        SCENE_QUERY_STAT(MVProjectileTargetSearch), false, Shooter);

    World->OverlapMultiByObjectType(
        Overlaps,
        SearchOrigin,
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        FCollisionShape::MakeSphere(Settings.SearchRadius),
        QueryParams);

    TSet<AMVCharacterBase*> Seen;
    TArray<AMVCharacterBase*> Candidates;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AMVCharacterBase* Candidate = Cast<AMVCharacterBase>(Overlap.GetActor());

        if (!IsValid(Candidate) || Seen.Contains(Candidate))
        {
            continue;
        }

        Seen.Add(Candidate);

        if (!CanAcquire(Shooter, Team, Candidate) ||
            FVector::DistSquared(
                SearchOrigin, Candidate->GetActorLocation()) > RadiusSquared)
        {
            continue;
        }

        if (Settings.Mode ==
                EMVProjectileTargetSelectionMode::NearestInFront &&
            FacingDot(Candidate) < MinimumFrontDot)
        {
            continue;
        }

        Candidates.Add(Candidate);
    }

    Algo::Sort(
        Candidates,
        [&](const AMVCharacterBase* Left,
            const AMVCharacterBase* Right)
        {
            const double LeftDistance = FVector::DistSquared(
                SearchOrigin, Left->GetActorLocation());
            const double RightDistance = FVector::DistSquared(
                SearchOrigin, Right->GetActorLocation());

            if (LeftDistance != RightDistance)
            {
                return LeftDistance < RightDistance;
            }

            const double LeftFacing = FacingDot(Left);
            const double RightFacing = FacingDot(Right);

            if (LeftFacing != RightFacing)
            {
                return LeftFacing > RightFacing;
            }

            return Left->GetUniqueID() < Right->GetUniqueID();
        });

    for (AMVCharacterBase* Candidate : Candidates)
    {
        if (CanUseTarget(Candidate))
        {
            return Candidate;
        }
    }

    return nullptr;
}