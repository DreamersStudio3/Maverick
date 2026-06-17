// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MVCharacterBase.h"

#include "Components/MVStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMVCharacterBase::AMVCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatComponent = CreateDefaultSubobject<UMVStatComponent>(TEXT("StatComponent"));
}

// Called when the game starts or when spawned
void AMVCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Character Moving Direction Speed Map
	FString CurvePath = "/Game/Miscellaneous/Curve/C_CharacterMoveSpeedMap.C_CharacterMoveSpeedMap";
	SpeedDirectionCurve = LoadObject<UCurveFloat>(nullptr, *CurvePath);

	// VERIFICATION: Check if it actually loaded
	if (SpeedDirectionCurve)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully loaded Curve: %s"), *SpeedDirectionCurve->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED to load Curve at path: %s. Check the path in Content Browser!"), *CurvePath);
	}
}

// Called every frame
void AMVCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateCharacterValue();
	UpdateRotation();
	UpdateMovement();


}

// Called to bind functionality to input
void AMVCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMVCharacterBase::AttemptCrouch()
{
	if (bIsFalling)
	{
		UnCrouch();
		return;
	}
	
	if (IsCrouched())
	{
		UnCrouch();
		return;
	}
	else
	{
		Crouch();
		return;
	}

}

void AMVCharacterBase::UpdateCharacterValue()
{
	// IsFalling
	bIsFalling = GetCharacterMovement()->IsFalling();
	
	// Calculate Move Direction Angle
	if (!GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		CharacterMoveDirectionAngle = UKismetAnimationLibrary::CalculateDirection(GetCharacterMovement()->Velocity, GetActorRotation());
		CharacterMoveDirectionAngleFromAcceleration = UKismetAnimationLibrary::CalculateDirection(GetCharacterMovement()->GetCurrentAcceleration(), GetActorRotation());
	}

	

	// HasMovementInput
	{
		FVector CurrentAcceleration2D = FVector(GetCharacterMovement()->GetCurrentAcceleration().X, GetCharacterMovement()->GetCurrentAcceleration().Y, 0);
		float Result = UKismetMathLibrary::SafeDivide(CurrentAcceleration2D.Length(), GetCharacterMovement()->GetMaxAcceleration());

		bHasMovementInput = (Result > 0.9);
	}

}

void AMVCharacterBase::UpdateRotation()
{
	if (CharacterInputState.WantsToStrafe || CharacterInputState.WantsToAim)
	{
		SetStrafeMode(true);
	}
	else
	{
		SetStrafeMode(false);
	}
}

void AMVCharacterBase::UpdateMovement()
{
	// Decide Gait
	Gait = DesiredGait();

	// Movement Speed
	const float WalkSpeed = StatComponent ? StatComponent->WalkSpeed : 200.0f;
	const float RunSpeed = StatComponent ? StatComponent->RunSpeed : 500.0f;
	const float SprintSpeed = StatComponent ? StatComponent->SprintSpeed : 750.0f;
	GetCharacterMovement()->MaxWalkSpeed = CalculateCharacterMovementSpeed(WalkSpeed, RunSpeed, SprintSpeed);

	// Acceleration
	GetCharacterMovement()->MaxAcceleration = 1000.0f;

	// Braking Deceleration
	if (bHasMovementInput)
	{
		GetCharacterMovement()->BrakingDecelerationWalking = 1000.0f;
	}
	else
	{
		GetCharacterMovement()->BrakingDecelerationWalking = 500.0f;
	}

	// Ground Friction
	GetCharacterMovement()->GroundFriction = 5.0f;

}

void AMVCharacterBase::SetStrafeMode(bool StrafeModeOn)
{
	GetCharacterMovement()->bUseControllerDesiredRotation = StrafeModeOn;
	GetCharacterMovement()->bOrientRotationToMovement = !StrafeModeOn;

	if (StrafeModeOn)
	{
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
	else
	{
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	}
}

EGait AMVCharacterBase::DesiredGait()
{
	if (CanSprint())
	{
		return EGait::Sprinting;
	}
	else
	{
		if (CharacterInputState.WantsToWalk)
		{
			return EGait::Walking;
		}
	}

	return EGait::Running;
}

bool AMVCharacterBase::CanSprint()
{
	bool StrafeCondition = true;
	if (CharacterInputState.WantsToStrafe)
	{
		//StrafeCondition = UKismetMathLibrary::InRange_FloatFloat(CharacterMoveDirectionAngle, -50, 50);
		StrafeCondition = false;
	}

	return
		(
			!IsCrouched() &&
			bHasMovementInput &&
			CharacterInputState.WantsToSprint &&
			!CharacterInputState.WantsToAim &&
			StrafeCondition
			);
}

float AMVCharacterBase::CalculateCharacterMovementSpeed(float WalkSpeed, float RunSpeed, float SprintSpeed)
{

	if (!SpeedDirectionCurve)
	{
		return RunSpeed;
	}
	float StrafeMapValue = SpeedDirectionCurve->GetFloatValue(abs(CharacterMoveDirectionAngle));

	float OutSpeed;
	switch (Gait)
	{
	case EGait::Walking:
		OutSpeed = WalkSpeed;
		break;
	case EGait::Running:
		OutSpeed = RunSpeed;
		break;
	case EGait::Sprinting:
		OutSpeed = SprintSpeed;
		break;
	default:
		OutSpeed = RunSpeed;
		break;
	}

	return UKismetMathLibrary::MapRangeClamped(StrafeMapValue, 0, 1, 0, OutSpeed);
}



