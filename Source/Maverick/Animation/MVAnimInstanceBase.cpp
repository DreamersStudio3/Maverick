// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/MVAnimInstanceBase.h"

#include "Character/MVCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMVAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	APawn* Owner = TryGetPawnOwner();

	if (!Owner)
	{
		return;
	}

	Character = Cast<AMVCharacterBase>(Owner);
	CharcterMovementComponent = Character->GetCharacterMovement();
}

void UMVAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr)
	{
		APawn* Owner = TryGetPawnOwner();

		if (!Owner)
		{
			return;
		}

		Character = Cast<AMVCharacterBase>(Owner);
		CharcterMovementComponent = Character->GetCharacterMovement();

		return;
	}

	ReceiveCharacterData();
	GetLocationData(DeltaSeconds);
	GetVelocityData();
	GetRotationData(DeltaSeconds);
	GetAccelerationData(DeltaSeconds);
	GetCharacterStateData();
	CalculatePivotState();


}

void UMVAnimInstanceBase::ReceiveCharacterData()
{
	IncomingGait = Character->Gait;
	CharacterInputState = Character->CharacterInputState;
}

void UMVAnimInstanceBase::GetLocationData(float DeltaTime)
{
	PreviousWorldLocation = CurrentWorldLocation;
	CurrentWorldLocation = Character->GetActorLocation();

	LocationDelta = (CurrentWorldLocation - PreviousWorldLocation).Length();
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(LocationDelta, DeltaTime);
}

void UMVAnimInstanceBase::GetVelocityData()
{
	PreviousVelocity = Velocity;
	Velocity = CharcterMovementComponent->Velocity;

	bHasVelocity2D = !UKismetMathLibrary::NearlyEqual_FloatFloat(FVector(Velocity.X, Velocity.Y, 0.0f).SquaredLength(), 0.0f, 0.01);

	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Length();
	MovingDirection = Character->CharacterMoveDirectionAngle;
	MovingDirectionFromAcceleration = Character->CharacterMoveDirectionAngleFromAcceleration;

	CalculateLocomotionDirection(MovingDirection, LocomotionDirection);

	CalculateLocomotionDirection(UKismetAnimationLibrary::CalculateDirection(CurrentAcceleration2D, ActorRotation), LocomotionDirectionFromAcceleration);
	

}

void UMVAnimInstanceBase::GetRotationData(float DeltaTime)
{
	ActorRotation = Character->GetActorRotation();
	
	PreActorYaw = ActorYaw;
	ActorYaw = ActorRotation.Yaw;
	ActorYawDelta = ActorYaw - PreActorYaw;

	float DividedAngle = UKismetMathLibrary::SafeDivide(ActorYawDelta, DeltaTime) * 0.25f;
	float Multiplier = 1.0f;
	if (FMath::Abs(MovingDirection) > 125)
	{
		Multiplier *= -1;
	}
	LeanAngle = UKismetMathLibrary::ClampAngle(DividedAngle, -90, 90) * Multiplier;

	FRotator DeltaRot = (Character->GetBaseAimRotation() - ActorRotation).GetNormalized();
	CharacterLookDirectionPitch = DeltaRot.Pitch;
	CharacterLookDirectionYaw = DeltaRot.Yaw;


}

void UMVAnimInstanceBase::GetAccelerationData(float DeltaTime)
{
	CurrentAcceleration = CharcterMovementComponent->GetCurrentAcceleration();
	PreviousAcceleration2D = CurrentAcceleration2D;
	CurrentAcceleration2D = FVector(CurrentAcceleration.X, CurrentAcceleration.Y, 0.0f);
	
	IsAccelerating = !UKismetMathLibrary::NearlyEqual_FloatFloat(CurrentAcceleration2D.SquaredLength(), 0.0f, 0.01f);

	// Relative Acceleration Missed (Optional)
	if (DeltaTime <= 0)
	{
		return;
	}
	FVector PhysicalAcceleration = (FVector(Velocity.X, Velocity.Y, 0.0f) - FVector(PreviousVelocity.X, PreviousVelocity.Y, 0.0f)) / DeltaTime;
	float ValueMultiplier;
	switch (CurrentGait)
	{
	case EGait::Walking:
		ValueMultiplier = 0.5f;
		break;
	case EGait::Running:
		ValueMultiplier = 0.7f;
		break;
	case EGait::Sprinting:
		ValueMultiplier = 1.0f;
		break;
	default:
		ValueMultiplier = 0.0f;
		break;
	}
	float MaxAccelerationSize;

	if (IsAccelerating)
	{
		MaxAccelerationSize = CharcterMovementComponent->GetMaxAcceleration();
	}
	else
	{
		MaxAccelerationSize = CharcterMovementComponent->GetMaxBrakingDeceleration();
	}

	if (MaxAccelerationSize > 0)
	{
		FVector ClampedPhysicalAcceleration = UKismetMathLibrary::Vector_ClampSizeMax(PhysicalAcceleration, MaxAccelerationSize);
		FVector UnrotatedVector = UKismetMathLibrary::Quat_UnrotateVector(ActorRotation.Quaternion(), (ClampedPhysicalAcceleration / MaxAccelerationSize));
		RelativeAcceleration = UnrotatedVector * ValueMultiplier;
		return;
	}
	else
	{
		RelativeAcceleration = FVector(0.0f, 0.0f, 0.0f);
		return;
	}

}

void UMVAnimInstanceBase::GetCharacterStateData()
{
	PreviousGait = CurrentGait;
	CurrentGait = IncomingGait;

	bGaitChanged = (CurrentGait != PreviousGait);
}

void UMVAnimInstanceBase::CalculatePivotState()
{
	FVector Acceleration2D = CurrentAcceleration.GetSafeNormal2D();
	FVector Velocity2D = Velocity.GetSafeNormal2D();

	if (Acceleration2D.IsNearlyZero() || Velocity2D.IsNearlyZero())
	{
		IsPivot = false;
		return;
	}
	
	float PivotDotValue = FVector::DotProduct(Acceleration2D, Velocity2D);

	IsPivot = PivotDotValue < 0.0f;
}

void UMVAnimInstanceBase::CalculateLocomotionDirection(float MoveDirectionAngle, ELocomotionDirection& Direction)
{
	float ABSAngle = FMath::Abs(MoveDirectionAngle);

	// If StrafeMode is false, Direction will be Forward
	if (!CharacterInputState.WantsToStrafe && !CharacterInputState.WantsToAim)
	{
		Direction = ELocomotionDirection::F;
		return;
	}

	// Backward Direction
	if (ABSAngle >= 112.5)
	{
		// Direction == Backward
		if (ABSAngle >= 157.5)
		{
			Direction = ELocomotionDirection::B;
			return;
		}
		// BackLeft or BackRight
		else
		{
			if (MoveDirectionAngle >= 0)
			{
				Direction = ELocomotionDirection::BR;
				return;
			}
			else
			{
				Direction = ELocomotionDirection::BL;
				return;
			}
		}
	}
	// Forward Direction
	else if (ABSAngle <= 67.5)
	{
		// Direction == Forward
		if (ABSAngle <= 22.5)
		{
			Direction = ELocomotionDirection::F;
			return;
		}
		// FrowardLeft or ForwardRight
		else
		{
			if (MoveDirectionAngle >= 0)
			{
				Direction = ELocomotionDirection::FR;
				return;
			}
			else
			{
				Direction = ELocomotionDirection::FL;
				return;
			}
		}
	}
	// Left or Right
	else
	{
		if (MoveDirectionAngle >= 0)
		{
			Direction = ELocomotionDirection::R;
			return;
		}
		else
		{
			Direction = ELocomotionDirection::L;
			return;
		}
	}
}

