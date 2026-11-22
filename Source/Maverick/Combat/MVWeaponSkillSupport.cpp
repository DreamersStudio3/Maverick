#include "Combat/MVWeaponSkillSupport.h"

#include "Character/MVCharacterBase.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Combat/Projectile/MVProjectileTargeting.h"
#include "Components/CapsuleComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"

namespace
{
constexpr float MVWeaponSkillConeCheckInterval = 0.05f;
constexpr double MVWeaponSkillConeHalfAngleCosine = 0.86602540378443865;

#if WITH_EDITOR && ENABLE_DRAW_DEBUG
DEFINE_LOG_CATEGORY_STATIC(LogMVWeaponSkillCone, Log, All);

TAutoConsoleVariable<int32> CVarMVWeaponSkillConeDebugDraw(
	TEXT("maverick.WeaponSkillCone.DebugDraw"),
	1,
	TEXT("Draw the weapon skill cone and report its hits in PIE. 0: off, 1: on."));

constexpr float MVWeaponSkillConeDebugSnapshotLifetime = 1.25f;

bool MVWeaponSkillConeShouldDraw(const UWorld& World)
{
	return World.WorldType == EWorldType::PIE && CVarMVWeaponSkillConeDebugDraw.GetValueOnGameThread() != 0;
}

void MVWeaponSkillDrawDebugCone(
	const UWorld& World, const FVector& Origin, const FVector& Forward,
	double Range, double HalfHeight, double AirborneRange, float Lifetime)
{
	constexpr int32 SegmentCount = 24;
	const double HalfAngle = FMath::Acos(MVWeaponSkillConeHalfAngleCosine);
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
	const FVector HeightOffset = FVector::UpVector * HalfHeight;
	const auto DirectionAt = [&](double Angle)
	{
		return Forward * FMath::Cos(Angle) + Right * FMath::Sin(Angle);
	};
	const auto DrawLine = [&](const FVector& Start, const FVector& End, const FColor& Color)
	{
		DrawDebugLine(&World, Start, End, Color, false, Lifetime, 0, 1.5f);
	};

	const FVector FirstDirection = DirectionAt(-HalfAngle);
	const FVector LastDirection = DirectionAt(HalfAngle);
	for (const FVector& PlaneOrigin : {Origin - HeightOffset, Origin + HeightOffset})
	{
		DrawLine(PlaneOrigin, PlaneOrigin + FirstDirection * Range, FColor::Cyan);
		DrawLine(PlaneOrigin, PlaneOrigin + LastDirection * Range, FColor::Cyan);
	}
	DrawLine(Origin - HeightOffset, Origin + HeightOffset, FColor::Cyan);
	for (const FVector& Edge : {FirstDirection, LastDirection, Forward})
	{
		const FVector Rim = Origin + Edge * Range;
		DrawLine(Rim - HeightOffset, Rim + HeightOffset, FColor::Cyan);
	}

	FVector PreviousDirection = FirstDirection;
	for (int32 Segment = 1; Segment <= SegmentCount; ++Segment)
	{
		const double Alpha = static_cast<double>(Segment) / SegmentCount;
		const FVector Direction = DirectionAt(FMath::Lerp(-HalfAngle, HalfAngle, Alpha));
		for (const FVector& PlaneOrigin : {Origin - HeightOffset, Origin + HeightOffset})
		{
			DrawLine(PlaneOrigin + PreviousDirection * Range, PlaneOrigin + Direction * Range, FColor::Cyan);
		}
		if (AirborneRange > 0.0 && AirborneRange <= Range)
		{
			DrawLine(Origin + PreviousDirection * AirborneRange, Origin + Direction * AirborneRange, FColor::Yellow);
		}
		PreviousDirection = Direction;
	}
}
#endif

void MVWeaponSkillApplyGroundPull(AMVCharacterBase* Victim, const FVector& Destination, float Duration)
{
	if (!IsValid(Victim) || !IsValid(Victim->StatComponent.Get()) || Victim->StatComponent->IsDead())
	{
		return;
	}

	UCharacterMovementComponent* Movement = Victim->GetCharacterMovement();
	const FVector Start = Victim->GetActorLocation();
	if (!IsValid(Movement) || FMath::IsNearlyZero(FVector::DistSquared2D(Start, Destination)))
	{
		return;
	}

	TSharedPtr<FRootMotionSource_ConstantForce> Pull = MakeShared<FRootMotionSource_ConstantForce>();
	Pull->InstanceName = TEXT("MVWeaponSkillGroundPull");
	Pull->Priority = 500;
	Pull->AccumulateMode = ERootMotionAccumulateMode::Override;
	Pull->Force = FVector(Destination.X - Start.X, Destination.Y - Start.Y, 0.0) / Duration;
	Pull->Duration = Duration;
	// 지면 높이는 보행 물리에 맡기고 마지막 프레임의 남은 시간만 적용한다.
	Pull->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	Pull->Settings.UnSetFlag(ERootMotionSourceSettingsFlags::DisablePartialEndTick);
	Pull->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	Pull->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	Movement->ApplyRootMotionSource(Pull);
}
}

UMVWeaponSkillConeWindow* UMVWeaponSkillConeWindow::OpenWeaponSkillCone(UMVAbilityBase* Ability, float Radius,
	UMVWeaponSkillConeWindow* ExistingWindow, float AirborneStartDistance)
{
	AMVCharacterBase* Owner = IsValid(Ability) ? Ability->GetOwnerCharacter() : nullptr;
	UWorld* World = IsValid(Owner) ? Owner->GetWorld() : nullptr;
	const UCapsuleComponent* Capsule = Owner ? Owner->GetCapsuleComponent() : nullptr;
	if (!World || !World->IsGameWorld() || !Capsule || !Ability->bAbilityActive || !(Radius > 0.0f))
	{
		return nullptr;
	}

	if (IsValid(ExistingWindow) && ExistingWindow->WindowWorld.IsValid()
		&& ExistingWindow->SourceAbility.Get() == Ability && ExistingWindow->AttackId == Ability->GetAttackInstanceId())
	{
		return ExistingWindow;
	}

	UMVWeaponSkillConeWindow* Window = NewObject<UMVWeaponSkillConeWindow>(Ability);
	Window->SourceAbility = Ability;
	Window->WindowWorld = World;
	Window->AttackId = Ability->GetAttackInstanceId();
	Window->Origin = Capsule->GetComponentLocation();
	Window->Forward = Owner->GetActorForwardVector().GetSafeNormal2D();
	Window->HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	Window->Range = Radius;
	Window->AirborneRange = FMath::Max(0.0f, AirborneStartDistance);

#if WITH_EDITOR && ENABLE_DRAW_DEBUG
	if (MVWeaponSkillConeShouldDraw(*World))
	{
		MVWeaponSkillDrawDebugCone(*World, Window->Origin, Window->Forward, Window->Range,
			Window->HalfHeight, Window->AirborneRange, MVWeaponSkillConeDebugSnapshotLifetime);
		UE_LOG(LogMVWeaponSkillCone, Display,
			TEXT("WeaponSkillConeTrace Begin Frame=%llu Owner=%s AttackId=%d Origin=%s Forward=%s Radius=%.1f HalfHeight=%.1f AirborneStart=%.1f"),
			GFrameCounter, *GetNameSafe(Owner), Window->AttackId, *Window->Origin.ToCompactString(),
			*Window->Forward.ToCompactString(), Window->Range, Window->HalfHeight, Window->AirborneRange);
	}
#endif

	World->GetTimerManager().SetTimer(Window->CheckTimer, Window,
		&UMVWeaponSkillConeWindow::CheckCone, MVWeaponSkillConeCheckInterval, true);
	Window->CheckCone();
	return Window;
}

void UMVWeaponSkillConeWindow::CloseWeaponSkillCone(UMVWeaponSkillConeWindow* Window)
{
	if (IsValid(Window))
	{
		Window->CloseCone();
	}
}

void UMVWeaponSkillConeWindow::CloseCone()
{
	if (UWorld* World = WindowWorld.Get())
	{
		World->GetTimerManager().ClearTimer(CheckTimer);
	}
	WindowWorld.Reset();
	HitCharacters.Reset();
}

void UMVWeaponSkillConeWindow::BeginDestroy()
{
	CloseCone();
	Super::BeginDestroy();
}

void UMVWeaponSkillConeWindow::CheckCone()
{
	UMVAbilityBase* Ability = SourceAbility.Get();
	AMVCharacterBase* Owner = Ability ? Ability->GetOwnerCharacter() : nullptr;
	UWorld* World = WindowWorld.Get();
	if (!World || !IsValid(Owner) || !IsValid(Owner->StatComponent.Get()) || Owner->StatComponent->IsDead()
		|| !Ability->bAbilityActive || Ability->GetAttackInstanceId() != AttackId)
	{
		CloseCone();
		return;
	}

	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(World);
	if (!Resolver)
	{
		return;
	}

#if WITH_EDITOR && ENABLE_DRAW_DEBUG
	const bool bDrawCone = MVWeaponSkillConeShouldDraw(*World);
	if (bDrawCone)
	{
		MVWeaponSkillDrawDebugCone(*World, Origin, Forward, Range, HalfHeight, AirborneRange, MVWeaponSkillConeCheckInterval * 2.0f);
	}
#endif

	TArray<FOverlapResult> Overlaps;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(MVWeaponSkillCone), false, Owner);
	World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeBox(FVector(Range, Range, HalfHeight)), Params);

	const EMVProjectileTeam Team = Owner->IsA<AMVPlayerCharacter>() ? EMVProjectileTeam::Player : EMVProjectileTeam::Enemy;
	const double AirborneDistanceSquared = FMath::Square(AirborneRange);
	FMVHitResolveRequest Request = UMVWeaponSkillSupport::MakeAreaHitRequest(*Ability, *Owner);
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (!Ability->bAbilityActive || Ability->GetAttackInstanceId() != AttackId)
		{
			break;
		}

		AMVCharacterBase* Victim = Cast<AMVCharacterBase>(Overlap.GetActor());
		if (!MVProjectileTargeting::CanAcquire(Owner, Team, Victim) || HitCharacters.Contains(Victim))
		{
			continue;
		}
		const FVector Center = Victim->GetCapsuleComponent()->GetComponentLocation();
		const FVector Offset = Center - Origin;
		const double DistanceSquared = Offset.SizeSquared2D();
		if (FMath::Abs(Offset.Z) > HalfHeight || DistanceSquared <= 0.0 || DistanceSquared > FMath::Square(Range)
			|| FVector::DotProduct(Offset, Forward) < FMath::Sqrt(DistanceSquared) * MVWeaponSkillConeHalfAngleCosine)
		{
			continue;
		}

		Request.Victim = Victim;
		Request.HitReactionType = DistanceSquared >= AirborneDistanceSquared
			? EMVActionHitReactionType::Airborne : EMVActionHitReactionType::Flinch;
		Request.HitLocation = Center;
		HitCharacters.Add(Victim);
		FMVResolvedHitData ResolvedHit;
		[[maybe_unused]] const bool bResolved = Resolver->ResolveAttackHit(Request, ResolvedHit);
#if WITH_EDITOR && ENABLE_DRAW_DEBUG
		if (bDrawCone)
		{
			const FColor HitColor = bResolved ? FColor::Green : (ResolvedHit.bBlocked ? FColor::Orange : FColor::Red);
			DrawDebugSphere(World, Center, 18.0f, 8, HitColor, false, MVWeaponSkillConeDebugSnapshotLifetime);
			UE_LOG(LogMVWeaponSkillCone, Display,
				TEXT("WeaponSkillConeTrace Hit Frame=%llu Owner=%s Target=%s AttackId=%d Distance=%.1f Request=%s Delivered=%d Blocked=%d"),
				GFrameCounter, *GetNameSafe(Owner), *GetNameSafe(Victim), AttackId, FMath::Sqrt(DistanceSquared),
				*UEnum::GetValueAsString(Request.HitReactionType), bResolved, ResolvedHit.bBlocked);
		}
#endif
	}
}

FMVHitResolveRequest UMVWeaponSkillSupport::MakeAreaHitRequest(const UMVAbilityBase& Ability, AMVCharacterBase& Owner)
{
	FMVHitResolveRequest Request;
	Request.Attacker = &Owner;
	Request.AttackInstanceId = Ability.GetAttackInstanceId();
	Request.AttackTypes = EMVAttackTypes::SkillAttack;
	Request.DamageMultiplier = Ability.AbilityData.DamageMultiplier;
	Request.GroggyDamageMultiplier = Ability.AbilityData.GroggyDamageMultiplier;
	if (const UMVWeaponComponent* Weapon = Owner.WeaponComponent.Get())
	{
		Request.PoiseDamage = Weapon->GetEquippedWeaponState().WeaponPoise * Ability.AbilityData.PoiseAmountMultiplier;
	}
	Request.HitLaunchData = Ability.GetHitLaunchData();
	return Request;
}

FMVProjectileSpawnInfo UMVWeaponSkillSupport::ResolveGroundPullSpawnInfo(UMVProjectileAbility* Ability)
{
	FMVProjectileSpawnInfo Info;
	if (AMVCharacterBase* Owner = IsValid(Ability) ? Ability->GetOwnerCharacter() : nullptr)
	{
		Info.Direction = Owner->GetActorForwardVector().GetSafeNormal2D();
		Info.Location = Owner->GetActorLocation() + 100.0 * Info.Direction + 30.0 * FVector::UpVector;
		Info.bValid = true;
	}
	return Info;
}

void UMVWeaponSkillSupport::ScheduleGroundPull(AMVProjectileBase* Projectile, const FMVResolvedHitData& HitData,
	float ForwardDistance, float Delay, float Duration)
{
	UWorld* World = IsValid(Projectile) ? Projectile->GetWorld() : nullptr;
	AMVCharacterBase* Victim = HitData.Victim.Get();
	if (!World || !IsValid(Victim) || HitData.bBlocked || !(Duration > 0.0f))
	{
		return;
	}

	const FMVProjectileLaunchContext& Context = Projectile->GetLaunchContext();
	const FVector Destination = Context.SourceLocation + Context.LaunchDirection.GetSafeNormal2D() * ForwardDistance;
	const FTimerDelegate Pull = FTimerDelegate::CreateWeakLambda(Victim,
		[Target = TWeakObjectPtr<AMVCharacterBase>(Victim), Destination, Duration]()
		{
			MVWeaponSkillApplyGroundPull(Target.Get(), Destination, Duration);
		});
	if (Delay > 0.0f)
	{
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, Pull, Delay, false);
	}
	else
	{
		World->GetTimerManager().SetTimerForNextTick(Pull);
	}
}
