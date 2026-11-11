#include "Combat/Boss/ElectricEelMan/MVElectricEelManEAbility.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVActionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Engine/World.h"

void UMVElectricEelManEAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	auto* Player = FindTarget();
	auto* Character = GetOwnerCharacter();
	if (!Player || !Character || !Player->InputManagerComponent
		|| Player->InputManagerComponent->IsGameplayInputBlocked()
		|| Character->GetDistanceTo(Player) > FMath::Max(1.0f, DashDistance) + FMath::Max(1.0f, GrabDistance)
		|| !BeginSkill(AbilityIndex)) return;
	Target = Player;
	GrabTime = -1.0f;
	bDamageDelivered = false;
	DashStart = Character->GetActorLocation();
	const FVector Direction = (Player->GetActorLocation() - DashStart).GetSafeNormal2D();
	DashEnd = DashStart + Direction * FMath::Min(FMath::Max(1.0f, DashDistance), FVector::Dist2D(DashStart, Player->GetActorLocation()));
	if (!Direction.IsNearlyZero()) Character->SetActorRotation(Direction.Rotation());
	LockOwnerMovement();
}
void UMVElectricEelManEAbility::TickSkill(float Elapsed)
{
	auto* Player = Target.Get();
	auto* Character = GetOwnerCharacter();
	if (!IsEligiblePlayer(Player))
	{
		EndAbility_Implementation();
		return;
	}
	if (GrabTime >= 0.0f)
	{
		if (!bDamageDelivered && Elapsed - GrabTime >= FMath::Max(0.0f, DamageDelay))
		{
			bDamageDelivered = true;
			HitPlayer(Player, 1.0f, EMVActionHitReactionType::None);
		}
		if (Elapsed - GrabTime >= FMath::Max(FMath::Max(0.05f, HoldDuration), DamageDelay))
			EndAbility_Implementation();
		return;
	}
	const float Alpha = FMath::Clamp(Elapsed / FMath::Max(0.05f, DashDuration), 0.0f, 1.0f);
	FHitResult Hit;
	Character->SetActorLocation(FMath::Lerp(DashStart, DashEnd, Alpha), true, &Hit);
	if (Character->GetDistanceTo(Player) <= FMath::Max(1.0f, GrabDistance))
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(EelGrabObstacle), false, Character);
		Params.AddIgnoredActor(Player);
		FCollisionObjectQueryParams Objects;
		Objects.AddObjectTypesToQuery(ECC_WorldStatic);
		Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
		FHitResult Wall;
		auto* Input = Player->InputManagerComponent.Get();
		if (!Input || Input->IsGameplayInputBlocked() || Character->GetWorld()->LineTraceSingleByObjectType(
			Wall, Character->GetActorLocation(), Player->GetActorLocation(), Objects, Params))
		{
			EndAbility_Implementation();
			return;
		}
		GrabTime = Elapsed;
		LockedInput = Input;
		Input->BeginGameplayInputBlock();
		VictimMovement = Player->GetCharacterMovement();
		VictimMode = VictimMovement->MovementMode;
		VictimCustomMode = VictimMovement->CustomMovementMode;
		VictimMovement->StopMovementImmediately();
		VictimMovement->DisableMovement();
		if (Player->ActionComponent) Player->ActionComponent->CancelActiveAction();
		UE_LOG(LogTemp, Log, TEXT("시퀀스 재생"));
		return;
	}
	if (Alpha >= 1.0f || Hit.bBlockingHit) EndAbility_Implementation();
}
void UMVElectricEelManEAbility::StopExecution()
{
	if (LockedInput.IsValid()) LockedInput->EndGameplayInputBlock();
	LockedInput.Reset();
	auto* Player = Target.Get();
	if (VictimMovement.IsValid() && IsValid(Player) && !Player->IsActorBeingDestroyed()
		&& Player->StatComponent && !Player->StatComponent->IsDead())
		VictimMovement->SetMovementMode(VictimMode, VictimCustomMode);
	VictimMovement.Reset();
	Target.Reset();
	GrabTime = -1.0f;
	Super::StopExecution();
}
