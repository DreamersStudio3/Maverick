// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MVCharacterBase.h"

#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVDeathComponent.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tags/MVGameplayTags.h"

namespace
{
FVector2D ClampCharacterControllerSpaceInput(const FVector2D& Input)
{
	const float SizeSquared = Input.SizeSquared();
	if (SizeSquared <= 1.0f)
	{
		return Input;
	}

	return Input / FMath::Sqrt(SizeSquared);
}

ELocomotionDirection ResolveCharacterEightWayDirection(const float MoveDirectionAngle)
{
	const float AbsAngle = FMath::Abs(MoveDirectionAngle);

	if (AbsAngle >= 157.5f)
	{
		return ELocomotionDirection::B;
	}

	if (AbsAngle >= 112.5f)
	{
		return MoveDirectionAngle >= 0.0f
			? ELocomotionDirection::BR
			: ELocomotionDirection::BL;
	}

	if (AbsAngle <= 22.5f)
	{
		return ELocomotionDirection::F;
	}

	if (AbsAngle <= 67.5f)
	{
		return MoveDirectionAngle >= 0.0f
			? ELocomotionDirection::FR
			: ELocomotionDirection::FL;
	}

	return MoveDirectionAngle >= 0.0f
		? ELocomotionDirection::R
		: ELocomotionDirection::L;
}

}

// Sets default values
AMVCharacterBase::AMVCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatComponent = CreateDefaultSubobject<UMVStatComponent>(TEXT("StatComponent"));
	ActionComponent = CreateDefaultSubobject<UMVActionComponent>(TEXT("ActionComponent"));
	CombatComponent = CreateDefaultSubobject<UMVCombatComponent>(TEXT("CombatComponent"));
	DeathComponent = CreateDefaultSubobject<UMVDeathComponent>(TEXT("DeathComponent"));
	HitReactionComponent = CreateDefaultSubobject<UMVHitReactionComponent>(TEXT("HitReactionComponent"));
	InputManagerComponent = CreateDefaultSubobject<UMVInputManagerComponent>(TEXT("InputManagerComponent"));
	WeaponComponent = CreateDefaultSubobject<UMVWeaponComponent>(TEXT("WeaponComponent"));
	CharacterIndexCode = MVGameplayTags::Character_Player_P1;
	ApplyCharacterIndexCodeToComponents();
	bHasDodgeMovementInput = false;
	LocomotionDirection = ELocomotionDirection::F;
}

void AMVCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ApplyCharacterIndexCodeToComponents();
	BindDamageHandlers();
}

// Called when the game starts or when spawned
void AMVCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	BindDamageHandlers();

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
	UpdateMovement(DeltaTime);
	UpdateRotation();


}

// Called to bind functionality to input
void AMVCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMVCharacterBase::AddMovementInput(const FVector WorldDirection, const float ScaleValue, const bool bForce)
{
	if (!bForce && StatComponent && StatComponent->IsDead())
	{
		return;
	}

	CacheControllerSpaceMovementInput(WorldDirection, ScaleValue);
	OnMovementInputReceived.Broadcast(WorldDirection * ScaleValue);
	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);
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

void AMVCharacterBase::SetCharacterIndexCode(const FGameplayTag NewCharacterIndexCode)
{
	CharacterIndexCode = NewCharacterIndexCode;
	ApplyCharacterIndexCodeToComponents();
}

FGameplayTag AMVCharacterBase::GetCharacterIndexCode() const
{
	return CharacterIndexCode;
}

bool AMVCharacterBase::HasDodgeMovementInput() const
{
	return bHasDodgeMovementInput;
}

bool AMVCharacterBase::TryGetControllerSpaceMovementInput(
	FVector2D& OutMovementInput,
	const int32 MaxFrameAge) const
{
	OutMovementInput = FVector2D::ZeroVector;
	if (ControllerSpaceMovementInput.IsNearlyZero())
	{
		return false;
	}

	const uint64 CurrentFrame = GFrameCounter;
	if (ControllerSpaceMovementInputFrame > CurrentFrame)
	{
		return false;
	}

	const uint64 FrameAge = CurrentFrame - ControllerSpaceMovementInputFrame;
	if (FrameAge > static_cast<uint64>(FMath::Max(0, MaxFrameAge)))
	{
		return false;
	}

	OutMovementInput = ControllerSpaceMovementInput;
	return true;
}

FRotator AMVCharacterBase::ResolveMovementInputReferenceRotation() const
{
	if (const AController* OwnerController = GetController())
	{
		return FRotator(0.0f, OwnerController->GetControlRotation().Yaw, 0.0f);
	}

	return FRotator(0.0f, GetActorRotation().Yaw, 0.0f);
}

FVector AMVCharacterBase::ResolveWorldDirectionFromControllerSpaceInput(
	const FVector2D& ControllerSpaceInput) const
{
	if (ControllerSpaceInput.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FRotator ReferenceRotation = ResolveMovementInputReferenceRotation();
	const FVector ForwardVector = FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
	const FVector RightVector = FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
	return (ForwardVector * ControllerSpaceInput.X + RightVector * ControllerSpaceInput.Y).GetSafeNormal2D();
}

void AMVCharacterBase::ApplyLocomotionDirectionSnapshot(const FVector& MovementDirection)
{
	const FVector MovementDirection2D(MovementDirection.X, MovementDirection.Y, 0.0f);
	if (MovementDirection2D.IsNearlyZero())
	{
		return;
	}

	CharacterMoveDirectionAngleFromAcceleration = UKismetAnimationLibrary::CalculateDirection(
		MovementDirection2D,
		GetActorRotation());
	UpdateLocomotionDirection();
}

void AMVCharacterBase::SetEquippedStyle(const EMVEquippedStyle NewEquippedStyle)
{
	EquippedStyle = NewEquippedStyle;
}

EMVEquippedStyle AMVCharacterBase::GetEquippedStyle() const
{
	return EquippedStyle;
}

bool AMVCharacterBase::IsMovementInputBlocked() const
{
	return InputManagerComponent && InputManagerComponent->IsMovementInputBlocked();
}

void AMVCharacterBase::BeginInvincibility()
{
	++InvincibilityCount;
}

void AMVCharacterBase::EndInvincibility()
{
	InvincibilityCount = FMath::Max(0, InvincibilityCount - 1);
}

bool AMVCharacterBase::IsInvincible() const
{
	return InvincibilityCount > 0;
}

bool AMVCharacterBase::OnHitResolved(const FMVResolvedHitData& HitData)
{
	if (HitData.VictimCharacterIndexCode.IsValid() && HitData.VictimCharacterIndexCode != CharacterIndexCode)
	{
		return false;
	}

	OnDamaged.Broadcast(HitData);
	return true;
}

void AMVCharacterBase::ApplyCharacterIndexCodeToComponents()
{
	if (ActionComponent)
	{
		ActionComponent->SetCharacterIndexCode(CharacterIndexCode);
	}

	if (StatComponent)
	{
		StatComponent->SetCharacterIndexCode(CharacterIndexCode);
	}
}

void AMVCharacterBase::BindDamageHandlers()
{
	if (StatComponent)
	{
		OnDamaged.RemoveDynamic(StatComponent, &UMVStatComponent::HandleDamaged);
		OnDamaged.AddUniqueDynamic(StatComponent, &UMVStatComponent::HandleDamaged);
	}
}

void AMVCharacterBase::UpdateCharacterValue()
{
	const bool bDead = StatComponent && StatComponent->IsDead();
	const bool bMovementInputBlocked = IsMovementInputBlocked() || bDead;

	// IsFalling
	bIsFalling = GetCharacterMovement()->IsFalling();
	
	// Calculate Move Direction Angle
	if (CharacterInputState.WantsToStrafe && !GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		CharacterMoveDirectionAngle = UKismetAnimationLibrary::CalculateDirection(GetCharacterMovement()->Velocity, GetActorRotation());
	}
	else
	{
		CharacterMoveDirectionAngle = 0;
	}

	const FVector CurrentAcceleration2D(
		GetCharacterMovement()->GetCurrentAcceleration().X,
		GetCharacterMovement()->GetCurrentAcceleration().Y,
		0.0f);
	CharacterMoveDirectionAngleFromAcceleration = CurrentAcceleration2D.IsNearlyZero(1.0f)
		? 0.0f
		: UKismetAnimationLibrary::CalculateDirection(CurrentAcceleration2D, GetActorRotation());

	

	// HasMovementInput
	{
		float Result = UKismetMathLibrary::SafeDivide(CurrentAcceleration2D.Length(), GetCharacterMovement()->GetMaxAcceleration());

		bHasMovementInput = (Result > 0.9);
	}

	if (bMovementInputBlocked)
	{
		bHasMovementInput = false;
	}

	bHasDodgeMovementInput = bHasMovementInput || !CurrentAcceleration2D.IsNearlyZero(1.0f);
	UpdateLocomotionDirection();
}

void AMVCharacterBase::UpdateLocomotionDirection()
{
	LocomotionDirection = ResolveCharacterEightWayDirection(CharacterMoveDirectionAngle);
	LocomotionDirectionFromAcceleration = ResolveCharacterEightWayDirection(CharacterMoveDirectionAngleFromAcceleration);
}

void AMVCharacterBase::CacheControllerSpaceMovementInput(
	const FVector& WorldDirection,
	const float ScaleValue)
{
	if (ControllerSpaceMovementInputFrame != GFrameCounter)
	{
		ControllerSpaceMovementInput = FVector2D::ZeroVector;
		ControllerSpaceMovementInputFrame = GFrameCounter;
	}

	const FVector2D ControllerSpaceInput = ResolveControllerSpaceMovementInput(WorldDirection, ScaleValue);
	if (!ControllerSpaceInput.IsNearlyZero())
	{
		ControllerSpaceMovementInput = ClampCharacterControllerSpaceInput(
			ControllerSpaceMovementInput + ControllerSpaceInput);
	}
}

FVector2D AMVCharacterBase::ResolveControllerSpaceMovementInput(
	const FVector& WorldDirection,
	const float ScaleValue) const
{
	const FVector WorldDirection2D(WorldDirection.X, WorldDirection.Y, 0.0f);
	if (WorldDirection2D.IsNearlyZero() || FMath::IsNearlyZero(ScaleValue))
	{
		return FVector2D::ZeroVector;
	}

	const FRotator ReferenceRotation = ResolveMovementInputReferenceRotation();
	const FVector ForwardVector = FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
	const FVector RightVector = FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
	const FVector ScaledWorldInput = WorldDirection2D.GetSafeNormal2D() * ScaleValue;
	return ClampCharacterControllerSpaceInput(FVector2D(
		FVector::DotProduct(ScaledWorldInput, ForwardVector),
		FVector::DotProduct(ScaledWorldInput, RightVector)));
}

void AMVCharacterBase::UpdateRotation()
{
	if (Gait == EGait::Sprinting)
	{
		SetStrafeMode(false);
	}
	else if ((bHasMovementInput || bIsFalling)
		&& (CharacterInputState.WantsToStrafe || CharacterInputState.WantsToAim))
	{
		SetStrafeMode(true);
	}
	else
	{
		SetStrafeMode(false);
	}
}

void AMVCharacterBase::UpdateMovement(float DeltaTime)
{
	// Decide Gait
	Gait = DesiredGait();
	UpdateRecoverableStats(DeltaTime);

	// Movement Speed
	const float WalkSpeed = StatComponent ? StatComponent->WalkSpeed : 200.0f;
	const float RunSpeed = StatComponent ? StatComponent->RunSpeed : 500.0f;
	const float SprintSpeed = StatComponent ? StatComponent->SprintSpeed : 750.0f;
	GetCharacterMovement()->MaxWalkSpeed = CalculateCharacterMovementSpeed(WalkSpeed, RunSpeed, SprintSpeed);

	// Acceleration
	const bool bMovementInputBlocked = IsMovementInputBlocked() || (StatComponent && StatComponent->IsDead());
	GetCharacterMovement()->MaxAcceleration = bMovementInputBlocked ? 0.0f : 1000.0f;

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

void AMVCharacterBase::UpdateRecoverableStats(float DeltaTime)
{
	if (!StatComponent || DeltaTime <= 0.0f)
	{
		return;
	}

	if (StatComponent->IsDead())
	{
		return;
	}

	StatComponent->TickRecoverableStats(DeltaTime);
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
		if (CharacterInputState.WantsToWalk || ShouldForceWalkGait())
		{
			return EGait::Walking;
		}
	}

	return EGait::Running;
}

bool AMVCharacterBase::CanSprint() const
{
	return
		(
			!IsCrouched() &&
			bHasMovementInput &&
			CharacterInputState.WantsToSprint &&
			!CharacterInputState.WantsToAim &&
			CanUseSprint()
			);
}

bool AMVCharacterBase::CanUseSprint() const
{
	return true;
}

bool AMVCharacterBase::ShouldForceWalkGait() const
{
	return false;
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



