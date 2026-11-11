#include "Combat/Boss/ElectricEelMan/MVElectricEelManWAbility.h"
#include "Character/MVCharacterBase.h"

void UMVElectricEelManWAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	auto* Target = FindTarget();
	auto* Character = GetOwnerCharacter();
	if (!Target || !Character || !BeginSkill(AbilityIndex)) return;
	JumpStart = Character->GetActorLocation();
	const FVector Offset = (Target->GetActorLocation() - JumpStart).GetSafeNormal2D();
	if (!Offset.IsNearlyZero()) Character->SetActorRotation(Offset.Rotation());
	const float Distance = FMath::Min(FVector::Dist2D(JumpStart, Target->GetActorLocation()), FMath::Max(0.0f, JumpDistance));
	JumpEnd = JumpStart + Offset * Distance;
	LockOwnerMovement();
}
void UMVElectricEelManWAbility::TickSkill(float Elapsed)
{
	auto* Character = GetOwnerCharacter();
	const float Alpha = FMath::Clamp(Elapsed / FMath::Max(0.05f, JumpDuration), 0.0f, 1.0f);
	FVector Location = FMath::Lerp(JumpStart, JumpEnd, Alpha);
	Location.Z += FMath::Max(0.0f, JumpHeight) * 4.0f * Alpha * (1.0f - Alpha);
	FHitResult Obstacle;
	Character->SetActorLocation(Location, true, &Obstacle);
	if (Alpha < 1.0f && !Obstacle.bBlockingHit) return;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	const FVector Center = Character->GetActorLocation() + Character->GetActorQuat().RotateVector(ImpactOffset);
	HitSphere(Center, FistRadius, 1.0f, EMVActionHitReactionType::Stagger, HitActors);
	HitSphere(Center, ShockwaveRadius, 0.5f, EMVActionHitReactionType::Flinch, HitActors);
	EndAbility_Implementation();
}
