#include "Components/MVDodgeComponent.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVActionComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tables/MVActionTableTypes.h"

namespace
{
FRotator MakeYawRotationFromDirection(const FVector& Direction)
{
	return FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);
}
}

UMVDodgeComponent::UMVDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVDodgeComponent::PrepareDodgeAction()
{
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	const FVector MovementInputDirection = ResolveMovementInputDirection(*OwnerCharacter);
	const bool bHasMovementInput = !MovementInputDirection.IsNearlyZero();

	if (bHasMovementInput && !OwnerCharacter->CharacterInputState.WantsToStrafe && !OwnerCharacter->CharacterInputState.WantsToAim)
	{
		OwnerCharacter->SetActorRotation(MakeYawRotationFromDirection(MovementInputDirection));
	}
	else if (OwnerCharacter->CharacterInputState.WantsToStrafe || OwnerCharacter->CharacterInputState.WantsToAim)
	{
		if (const AController* Controller = OwnerCharacter->GetController())
		{
			const FRotator ControlRotation = Controller->GetControlRotation();
			OwnerCharacter->SetActorRotation(FRotator(0.0f, ControlRotation.Yaw, 0.0f));
		}
	}

	OwnerCharacter->RefreshDodgeChooserData();

	PreparedDodgeLaunchDirection = bHasMovementInput
		? MovementInputDirection.GetSafeNormal2D()
		: -OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	bHasPreparedDodgeLaunchDirection = !PreparedDodgeLaunchDirection.IsNearlyZero();
}

bool UMVDodgeComponent::BeginDodgeLaunchWindow(
	const float NotifyDuration,
	UCurveFloat* DistanceCurve,
	const float DistanceScale,
	const bool bApplyVerticalLaunch)
{
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return false;
	}

	UMVActionComponent* ActionComponent = OwnerCharacter->FindComponentByClass<UMVActionComponent>();
	if (!ActionComponent)
	{
		return false;
	}

	StopActiveDodgeLaunch(false);

	const FMVActionStatRow* ActionStat = ActionComponent->FindActionStatRow(
		ActionComponent->GetActiveActionId(),
		EMVActionType::Dodge);
	if (!ActionStat)
	{
		return false;
	}

	const float LaunchDistance = ResolveDodgeLaunchDistance(*ActionStat, *OwnerCharacter);
	const float LaunchDuration = ResolveDodgeLaunchDuration(*ActionStat, NotifyDuration);
	const float ScaledLaunchDistance = LaunchDistance * FMath::Max(0.0f, DistanceScale);
	const float LaunchVerticalSpeed = ActionStat->LaunchVerticalSpeed;
	if ((ScaledLaunchDistance <= 0.0f || LaunchDuration <= KINDA_SMALL_NUMBER)
		&& (!bApplyVerticalLaunch || FMath::IsNearlyZero(LaunchVerticalSpeed)))
	{
		return false;
	}

	const FVector LaunchDirection = ResolveDodgeLaunchDirection(*OwnerCharacter);
	bHasPreparedDodgeLaunchDirection = false;
	PreparedDodgeLaunchDirection = FVector::ZeroVector;
	if (ScaledLaunchDistance > 0.0f && LaunchDuration > KINDA_SMALL_NUMBER && LaunchDirection.IsNearlyZero())
	{
		return false;
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		FVector Velocity = MovementComponent->Velocity;
		Velocity.X = 0.0f;
		Velocity.Y = 0.0f;
		MovementComponent->Velocity = Velocity;
	}

	if (ScaledLaunchDistance > 0.0f && LaunchDuration > KINDA_SMALL_NUMBER)
	{
		ActiveDodgeLaunchDistanceCurve = DistanceCurve;
		ActiveDodgeLaunchCharacter = OwnerCharacter;
		ActiveDodgeLaunchDirection = LaunchDirection;
		ActiveDodgeLaunchTargetDistance = ScaledLaunchDistance;
		ActiveDodgeLaunchDuration = LaunchDuration;
		ActiveDodgeLaunchElapsed = 0.0f;
		ActiveDodgeLaunchLastAlpha = 0.0f;
		bDodgeLaunchActive = true;
	}

	if (bApplyVerticalLaunch && !FMath::IsNearlyZero(LaunchVerticalSpeed))
	{
		OwnerCharacter->LaunchCharacter(FVector(0.0f, 0.0f, LaunchVerticalSpeed), false, true);
	}

	return true;
}

void UMVDodgeComponent::TickDodgeLaunchWindow(const float DeltaTime)
{
	if (!bDodgeLaunchActive || DeltaTime <= 0.0f)
	{
		return;
	}

	AMVCharacterBase* OwnerCharacter = ActiveDodgeLaunchCharacter.Get();
	if (!OwnerCharacter)
	{
		StopActiveDodgeLaunch(false);
		return;
	}

	ActiveDodgeLaunchElapsed += DeltaTime;

	const float NormalizedTime = ActiveDodgeLaunchDuration > KINDA_SMALL_NUMBER
		? FMath::Clamp(ActiveDodgeLaunchElapsed / ActiveDodgeLaunchDuration, 0.0f, 1.0f)
		: 1.0f;
	const float DistanceAlpha = FMath::Max(
		ActiveDodgeLaunchLastAlpha,
		EvaluateDodgeLaunchDistanceAlpha(NormalizedTime));
	const float DeltaDistance = FMath::Max(
		0.0f,
		(DistanceAlpha - ActiveDodgeLaunchLastAlpha) * ActiveDodgeLaunchTargetDistance);

	if (DeltaDistance > KINDA_SMALL_NUMBER)
	{
		FHitResult SweepHit;
		OwnerCharacter->AddActorWorldOffset(ActiveDodgeLaunchDirection * DeltaDistance, true, &SweepHit);
		if (SweepHit.bBlockingHit)
		{
			StopActiveDodgeLaunch(true);
			return;
		}
	}

	ActiveDodgeLaunchLastAlpha = DistanceAlpha;

	if (NormalizedTime >= 1.0f || DistanceAlpha >= 1.0f - KINDA_SMALL_NUMBER)
	{
		StopActiveDodgeLaunch(false);
	}
}

void UMVDodgeComponent::EndDodgeLaunchWindow(const bool bClearHorizontalVelocity)
{
	StopActiveDodgeLaunch(bClearHorizontalVelocity);
}

void UMVDodgeComponent::StopActiveDodgeLaunch(const bool bClearHorizontalVelocity)
{
	AMVCharacterBase* OwnerCharacter = ActiveDodgeLaunchCharacter.Get();
	if (bDodgeLaunchActive && bClearHorizontalVelocity && OwnerCharacter)
	{
		OwnerCharacter->LaunchCharacter(FVector::ZeroVector, true, false);
	}

	ActiveDodgeLaunchCharacter = nullptr;
	ActiveDodgeLaunchDistanceCurve = nullptr;
	PreparedDodgeLaunchDirection = FVector::ZeroVector;
	ActiveDodgeLaunchDirection = FVector::ZeroVector;
	ActiveDodgeLaunchTargetDistance = 0.0f;
	ActiveDodgeLaunchDuration = 0.0f;
	ActiveDodgeLaunchElapsed = 0.0f;
	ActiveDodgeLaunchLastAlpha = 0.0f;
	bHasPreparedDodgeLaunchDirection = false;
	bDodgeLaunchActive = false;
}

FVector UMVDodgeComponent::ResolveMovementInputDirection(const AMVCharacterBase& OwnerCharacter) const
{
	const UCharacterMovementComponent* MovementComponent = OwnerCharacter.GetCharacterMovement();
	const FVector Acceleration2D = MovementComponent
		? FVector(MovementComponent->GetCurrentAcceleration().X, MovementComponent->GetCurrentAcceleration().Y, 0.0f)
		: FVector::ZeroVector;

	if (!Acceleration2D.IsNearlyZero(1.0f))
	{
		return Acceleration2D.GetSafeNormal2D();
	}

	const FVector PendingInput2D(
		OwnerCharacter.GetPendingMovementInputVector().X,
		OwnerCharacter.GetPendingMovementInputVector().Y,
		0.0f);
	if (!PendingInput2D.IsNearlyZero(1.0f))
	{
		return PendingInput2D.GetSafeNormal2D();
	}

	const FVector LastInput2D(
		OwnerCharacter.GetLastMovementInputVector().X,
		OwnerCharacter.GetLastMovementInputVector().Y,
		0.0f);
	return LastInput2D.GetSafeNormal2D();
}

FVector UMVDodgeComponent::ResolveDodgeLaunchDirection(const AMVCharacterBase& OwnerCharacter) const
{
	if (bHasPreparedDodgeLaunchDirection && !PreparedDodgeLaunchDirection.IsNearlyZero())
	{
		return PreparedDodgeLaunchDirection.GetSafeNormal2D();
	}

	const FVector Acceleration2D = ResolveMovementInputDirection(OwnerCharacter);
	const bool bBackstep = !OwnerCharacter.bHasDodgeMovementInput;
	FVector LaunchDirection = Acceleration2D.GetSafeNormal2D();
	if (LaunchDirection.IsNearlyZero())
	{
		switch (OwnerCharacter.LocomotionDirection)
		{
		case ELocomotionDirection::FR:
			LaunchDirection = (OwnerCharacter.GetActorForwardVector() + OwnerCharacter.GetActorRightVector()).GetSafeNormal2D();
			break;
		case ELocomotionDirection::R:
			LaunchDirection = OwnerCharacter.GetActorRightVector().GetSafeNormal2D();
			break;
		case ELocomotionDirection::BR:
			LaunchDirection = (-OwnerCharacter.GetActorForwardVector() + OwnerCharacter.GetActorRightVector()).GetSafeNormal2D();
			break;
		case ELocomotionDirection::B:
			LaunchDirection = -OwnerCharacter.GetActorForwardVector().GetSafeNormal2D();
			break;
		case ELocomotionDirection::BL:
			LaunchDirection = (-OwnerCharacter.GetActorForwardVector() - OwnerCharacter.GetActorRightVector()).GetSafeNormal2D();
			break;
		case ELocomotionDirection::L:
			LaunchDirection = -OwnerCharacter.GetActorRightVector().GetSafeNormal2D();
			break;
		case ELocomotionDirection::FL:
			LaunchDirection = (OwnerCharacter.GetActorForwardVector() - OwnerCharacter.GetActorRightVector()).GetSafeNormal2D();
			break;
		case ELocomotionDirection::F:
		default:
			LaunchDirection = OwnerCharacter.GetActorForwardVector().GetSafeNormal2D();
			break;
		}
	}

	if (bBackstep)
	{
		LaunchDirection = -OwnerCharacter.GetActorForwardVector().GetSafeNormal2D();
	}

	return LaunchDirection.GetSafeNormal2D();
}

float UMVDodgeComponent::ResolveDodgeLaunchDistance(
	const FMVActionStatRow& ActionStat,
	const AMVCharacterBase& OwnerCharacter) const
{
	const float DefaultDistance = FMath::Max(0.0f, ActionStat.LaunchDistance);
	return OwnerCharacter.bHasDodgeMovementInput
		? DefaultDistance
		: DefaultDistance * 0.5f;
}

float UMVDodgeComponent::ResolveDodgeLaunchDuration(
	const FMVActionStatRow& ActionStat,
	const float NotifyDuration) const
{
	return ActionStat.LaunchDuration > 0.0f
		? ActionStat.LaunchDuration
		: FMath::Max(0.0f, NotifyDuration);
}

float UMVDodgeComponent::EvaluateDodgeLaunchDistanceAlpha(const float NormalizedTime) const
{
	const float ClampedTime = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
	if (ActiveDodgeLaunchDistanceCurve)
	{
		return FMath::Clamp(ActiveDodgeLaunchDistanceCurve->GetFloatValue(ClampedTime), 0.0f, 1.0f);
	}

	return ClampedTime * ClampedTime * (3.0f - 2.0f * ClampedTime);
}
