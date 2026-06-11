// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MVCharacterBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMVCharacterBase::AMVCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMVCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
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
	CharacterMoveDirectionAngle = UKismetAnimationLibrary::CalculateDirection(GetCharacterMovement()->Velocity, GetActorRotation());

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

	// Todo: Update Accleration, Braking Deceleration, GroundFriction, Calculate MaxWalkSpeed along with moveing direction (optional: Crouch)

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
		StrafeCondition = UKismetMathLibrary::InRange_FloatFloat(CharacterMoveDirectionAngle, -50, 50);
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



