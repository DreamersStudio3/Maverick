// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MVCharacterBase.h"

#include "Components/MVActionComponent.h"
#include "Components/MVDodgeComponent.h"
#include "Components/MVStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"

namespace
{
ELocomotionDirection ResolveEightWayDirection(const float MoveDirectionAngle)
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
	DodgeComponent = CreateDefaultSubobject<UMVDodgeComponent>(TEXT("DodgeComponent"));
	bIsSprintBlockedByStamina = false;
	bHasDodgeMovementInput = false;
	LocomotionDirection = ELocomotionDirection::F;
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

	if (ActionComponent)
	{
		ActionComponent->OnActionStatRecoveryPauseChanged.AddUniqueDynamic(
			this,
			&AMVCharacterBase::HandleActionStatRecoveryPauseChanged);
		ActionComponent->OnActionPreparing.AddUniqueDynamic(
			this,
			&AMVCharacterBase::HandleActionPreparing);
		ActionComponent->OnActionCostConsumed.AddUniqueDynamic(
			this,
			&AMVCharacterBase::HandleActionCostConsumed);
	}

	CacheSprintActionData();
}

// Called every frame
void AMVCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateCharacterValue();
	UpdateRotation();
	UpdateMovement(DeltaTime);


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

bool AMVCharacterBase::HasDodgeMovementInput() const
{
	if (bHasMovementInput)
	{
		return true;
	}

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return false;
	}

	const FVector Acceleration2D(
		MovementComponent->GetCurrentAcceleration().X,
		MovementComponent->GetCurrentAcceleration().Y,
		0.0f);
	return !Acceleration2D.IsNearlyZero(1.0f);
}

void AMVCharacterBase::RefreshDodgeChooserData()
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const FVector CurrentAcceleration2D = MovementComponent
		? FVector(MovementComponent->GetCurrentAcceleration().X, MovementComponent->GetCurrentAcceleration().Y, 0.0f)
		: FVector::ZeroVector;
	if (!CurrentAcceleration2D.IsNearlyZero(1.0f))
	{
		CharacterMoveDirectionAngleFromAcceleration =
			UKismetAnimationLibrary::CalculateDirection(CurrentAcceleration2D, GetActorRotation());
	}

	if (MovementComponent)
	{
		const float InputRatio = UKismetMathLibrary::SafeDivide(
			CurrentAcceleration2D.Length(),
			MovementComponent->GetMaxAcceleration());
		bHasMovementInput = InputRatio > 0.9f;
	}

	bHasDodgeMovementInput = HasDodgeMovementInput();
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

void AMVCharacterBase::BeginMovementInputBlock()
{
	++MovementInputBlockCount;
}

void AMVCharacterBase::EndMovementInputBlock()
{
	MovementInputBlockCount = FMath::Max(0, MovementInputBlockCount - 1);
}

bool AMVCharacterBase::IsMovementInputBlocked() const
{
	return MovementInputBlockCount > 0;
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

void AMVCharacterBase::UpdateCharacterValue()
{
	const bool bMovementInputBlocked = IsMovementInputBlocked();

	// IsFalling
	bIsFalling = GetCharacterMovement()->IsFalling();
	
	// Calculate Move Direction Angle
	if (!GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		CharacterMoveDirectionAngle = UKismetAnimationLibrary::CalculateDirection(GetCharacterMovement()->Velocity, GetActorRotation());
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

	RefreshDodgeChooserData();
	UpdateLocomotionDirection();
}

void AMVCharacterBase::UpdateLocomotionDirection()
{
	LocomotionDirection = ResolveEightWayDirection(CharacterMoveDirectionAngleFromAcceleration);
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
	const bool bMovementInputBlocked = IsMovementInputBlocked();
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

	const float SprintStaminaCostPerSecond = ResolveSprintStaminaCostPerSecond();
	if (SprintStaminaCostPerSecond <= 0.0f)
	{
		bIsSprintBlockedByStamina = false;
	}

	const bool bShouldConsumeStamina = SprintStaminaCostPerSecond > 0.0f && Gait == EGait::Sprinting && bHasMovementInput;
	if (bShouldConsumeStamina)
	{
		RestartRecoverableStatCooldown();
		StatComponent->ConsumeStamina(CalculateSprintStaminaDrain(DeltaTime));

		if (StatComponent->CurrentStamina <= KINDA_SMALL_NUMBER)
		{
			StatComponent->SetCurrentStamina(0.0f);
			bIsSprintBlockedByStamina = true;
		}
		return;
	}

	if (ActionComponent && ActionComponent->IsActionStatRecoveryPaused())
	{
		return;
	}

	if (bUseStaminaRecoveryDelay)
	{
		RecoverableStatCooldownRemaining = FMath::Max(0.0f, RecoverableStatCooldownRemaining - DeltaTime);
	}
	else
	{
		RecoverableStatCooldownRemaining = 0.0f;
	}

	if (RecoverableStatCooldownRemaining <= 0.0f)
	{
		StatComponent->RecoverStamina(StatComponent->StaminaRecoveryPerSecond * DeltaTime);
		StatComponent->RecoverMP(StatComponent->MPRecoveryPerSecond * DeltaTime);
	}

	const float ResumeThreshold = StatComponent->MaxStamina * ResolveSprintResumeStaminaRatio();
	if (bIsSprintBlockedByStamina && StatComponent->CurrentStamina >= ResumeThreshold)
	{
		bIsSprintBlockedByStamina = false;
	}
}

void AMVCharacterBase::CacheSprintActionData()
{
	bHasSprintActionData = false;
	SprintActionStaminaCost = FMath::Max(0.0f, FallbackSprintStaminaCost);
	SprintActionStaminaCostType = EMVActionResourceCostType::PerSecond;
	SprintActionMinRequiredStamina = 0.0f;
	SprintActionRestartStaminaPercent = FMath::Clamp(SprintResumeStaminaRatio * 100.0f, 0.0f, 100.0f);

	const int32 SprintRawActionId = MVActionIds::ToRawActionId(SprintActionId);
	const FMVActionStatRow* SprintActionStat = ActionComponent
		? ActionComponent->FindActionStatRow(SprintRawActionId, EMVActionType::Sprint)
		: nullptr;
	if (!SprintActionStat)
	{
		return;
	}

	SprintActionStaminaCost = FMath::Max(0.0f, SprintActionStat->StaminaCost);
	SprintActionStaminaCostType = SprintActionStat->StaminaCostType;
	SprintActionMinRequiredStamina = FMath::Max(0.0f, SprintActionStat->MinRequiredStamina);
	SprintActionRestartStaminaPercent = SprintActionStat->SprintRestartStaminaPercent > 0.0f
		? FMath::Clamp(SprintActionStat->SprintRestartStaminaPercent, 0.0f, 100.0f)
		: FMath::Clamp(SprintResumeStaminaRatio * 100.0f, 0.0f, 100.0f);
	bHasSprintActionData = true;
}

void AMVCharacterBase::RestartRecoverableStatCooldown()
{
	RecoverableStatCooldownRemaining = bUseStaminaRecoveryDelay && StatComponent
		? StatComponent->StaminaRecoveryDelay
		: 0.0f;
}

void AMVCharacterBase::HandleActionStatRecoveryPauseChanged(bool bPaused)
{
	if (bPaused)
	{
		RecoverableStatCooldownRemaining = 0.0f;
	}
	else
	{
		RestartRecoverableStatCooldown();
	}

	OnStatRecentLossHoldChanged.Broadcast(bPaused);
}

void AMVCharacterBase::HandleActionPreparing(const int32 ActionId)
{
	if (ActionId == MVActionIds::Dodge && DodgeComponent)
	{
		DodgeComponent->PrepareDodgeAction();
	}
}

void AMVCharacterBase::HandleActionCostConsumed(int32 ActionId)
{
	RestartRecoverableStatCooldown();
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

float AMVCharacterBase::CalculateSprintStaminaDrain(float DeltaTime) const
{
	if (!StatComponent || DeltaTime <= 0.0f)
	{
		return 0.0f;
	}

	switch (SprintActionStaminaCostType)
	{
	case EMVActionResourceCostType::PerSecond:
		return StatComponent->MaxStamina * SprintActionStaminaCost / 100.0f * DeltaTime;
	case EMVActionResourceCostType::None:
	case EMVActionResourceCostType::Instant:
	case EMVActionResourceCostType::OnDemand:
	default:
		return 0.0f;
	}
}

float AMVCharacterBase::ResolveSprintStaminaCostPerSecond() const
{
	if (!StatComponent)
	{
		return SprintActionStaminaCost;
	}

	switch (SprintActionStaminaCostType)
	{
	case EMVActionResourceCostType::PerSecond:
		return StatComponent->MaxStamina * SprintActionStaminaCost / 100.0f;
	case EMVActionResourceCostType::None:
	case EMVActionResourceCostType::Instant:
	case EMVActionResourceCostType::OnDemand:
	default:
		return 0.0f;
	}
}

float AMVCharacterBase::ResolveSprintMinRequiredStamina() const
{
	return SprintActionMinRequiredStamina;
}

float AMVCharacterBase::ResolveSprintResumeStaminaRatio() const
{
	const float RestartPercent = bHasSprintActionData
		? SprintActionRestartStaminaPercent
		: SprintResumeStaminaRatio * 100.0f;
	return FMath::Clamp(RestartPercent / 100.0f, 0.0f, 1.0f);
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
			StrafeCondition &&
			CanUseSprintStamina()
			);
}

bool AMVCharacterBase::CanUseSprintStamina() const
{
	if (!StatComponent)
	{
		return true;
	}

	if (bIsSprintBlockedByStamina)
	{
		return false;
	}

	const float SprintStaminaCostPerSecond = ResolveSprintStaminaCostPerSecond();
	return StatComponent->CurrentStamina >= ResolveSprintMinRequiredStamina()
		&& (SprintStaminaCostPerSecond <= 0.0f || StatComponent->CurrentStamina > KINDA_SMALL_NUMBER);
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



