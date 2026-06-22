#include "Components/MVDodgeComponent.h"

#include "Character/MVCharacterBase.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/MVActionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tables/MVActionTableTypes.h"

namespace
{
constexpr float NonRollDodgeLaunchDistanceScale = 0.75f;

FVector2D DodgeClampControllerSpaceInput(const FVector2D& Input)
{
	const float SizeSquared = Input.SizeSquared();
	if (SizeSquared <= 1.0f)
	{
		return Input;
	}

	return Input / FMath::Sqrt(SizeSquared);
}

FVector DodgeControllerSpaceInputToVector(const FVector2D& Input)
{
	return FVector(Input.X, Input.Y, 0.0f);
}

FVector2D DodgeControllerSpaceInputFromVector(const FVector& Input)
{
	return DodgeClampControllerSpaceInput(FVector2D(Input.X, Input.Y));
}

FRotator MakeYawRotationFromDirection(const FVector& Direction)
{
	return FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);
}

FVector NormalizeMovementInputDirection(const FVector& Direction)
{
	const FVector Direction2D(Direction.X, Direction.Y, 0.0f);
	return Direction2D.IsNearlyZero()
		? FVector::ZeroVector
		: Direction2D.GetSafeNormal2D();
}

bool IsForwardDiagonalDodgeDirection(const ELocomotionDirection Direction)
{
	return Direction == ELocomotionDirection::FL
		|| Direction == ELocomotionDirection::FR;
}

bool IsBackwardDiagonalDodgeDirection(const ELocomotionDirection Direction)
{
	return Direction == ELocomotionDirection::BL
		|| Direction == ELocomotionDirection::BR;
}

ELocomotionDirection ResolveDodgeEightWayDirection(const float MoveDirectionAngle)
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

ELocomotionDirection DodgeResolveDirectionFromControllerSpaceInput(const FVector2D& ControllerSpaceInput)
{
	if (ControllerSpaceInput.IsNearlyZero())
	{
		return ELocomotionDirection::F;
	}

	const FVector2D NormalizedInput = ControllerSpaceInput.GetSafeNormal();
	const float DirectionAngle = FMath::RadiansToDegrees(FMath::Atan2(NormalizedInput.Y, NormalizedInput.X));
	return ResolveDodgeEightWayDirection(DirectionAngle);
}

FVector GetReferenceForwardVector(const FRotator& ReferenceRotation)
{
	return FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
}

FVector GetReferenceRightVector(const FRotator& ReferenceRotation)
{
	return FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
}

FVector ResolveDirectionVectorFromReferenceRotation(
	const ELocomotionDirection Direction,
	const FRotator& ReferenceRotation)
{
	const FVector ForwardVector = GetReferenceForwardVector(ReferenceRotation);
	const FVector RightVector = GetReferenceRightVector(ReferenceRotation);

	switch (Direction)
	{
	case ELocomotionDirection::FR:
		return (ForwardVector + RightVector).GetSafeNormal2D();
	case ELocomotionDirection::R:
		return RightVector;
	case ELocomotionDirection::BR:
		return (-ForwardVector + RightVector).GetSafeNormal2D();
	case ELocomotionDirection::B:
		return -ForwardVector;
	case ELocomotionDirection::BL:
		return (-ForwardVector - RightVector).GetSafeNormal2D();
	case ELocomotionDirection::L:
		return -RightVector;
	case ELocomotionDirection::FL:
		return (ForwardVector - RightVector).GetSafeNormal2D();
	case ELocomotionDirection::F:
	default:
		return ForwardVector;
	}
}

FVector DodgeResolveWorldDirectionFromControllerSpaceInput(
	const FVector2D& ControllerSpaceInput,
	const FRotator& ReferenceRotation)
{
	if (ControllerSpaceInput.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FVector ForwardVector = GetReferenceForwardVector(ReferenceRotation);
	const FVector RightVector = GetReferenceRightVector(ReferenceRotation);
	return (ForwardVector * ControllerSpaceInput.X + RightVector * ControllerSpaceInput.Y).GetSafeNormal2D();
}

FVector2D DodgeResolveControllerSpaceInputFromWorldDirection(
	const FVector& WorldDirection,
	const FRotator& ReferenceRotation)
{
	const FVector WorldDirection2D = NormalizeMovementInputDirection(WorldDirection);
	if (WorldDirection2D.IsNearlyZero())
	{
		return FVector2D::ZeroVector;
	}

	return DodgeClampControllerSpaceInput(FVector2D(
		FVector::DotProduct(WorldDirection2D, GetReferenceForwardVector(ReferenceRotation)),
		FVector::DotProduct(WorldDirection2D, GetReferenceRightVector(ReferenceRotation))));
}

FRotator ResolveStrafeReferenceRotation(const AMVCharacterBase& OwnerCharacter)
{
	return OwnerCharacter.ResolveMovementInputReferenceRotation();
}

FVector ResolveDodgeFacingDirection(
	const FVector& MovementInputDirection,
	const bool bHasMovementInput,
	const bool bFreeDodge,
	const ELocomotionDirection StrafeInputDirection)
{
	if (!bHasMovementInput)
	{
		return FVector::ZeroVector;
	}

	if (bFreeDodge || IsForwardDiagonalDodgeDirection(StrafeInputDirection))
	{
		return MovementInputDirection;
	}

	if (IsBackwardDiagonalDodgeDirection(StrafeInputDirection))
	{
		return -MovementInputDirection;
	}

	return FVector::ZeroVector;
}

int32 ResolveActiveMontageInstanceId(
	const AMVCharacterBase& OwnerCharacter,
	const UMVActionComponent& ActionComponent)
{
	const USkeletalMeshComponent* Mesh = OwnerCharacter.GetMesh();
	UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	const UAnimMontage* ActiveMontage = ActionComponent.GetActiveActionMontage();
	const FAnimMontageInstance* MontageInstance = AnimInstance && ActiveMontage
		? AnimInstance->GetActiveInstanceForMontage(ActiveMontage)
		: nullptr;
	return MontageInstance ? MontageInstance->GetInstanceID() : INDEX_NONE;
}

FVector ResolveGroundAdjustedLaunchDelta(
	const AMVCharacterBase& OwnerCharacter,
	const FVector& LaunchDirection,
	const float DeltaDistance)
{
	const FVector HorizontalDelta = LaunchDirection.GetSafeNormal2D() * DeltaDistance;
	const UCharacterMovementComponent* MovementComponent = OwnerCharacter.GetCharacterMovement();
	if (!MovementComponent
		|| !MovementComponent->IsMovingOnGround()
		|| !MovementComponent->CurrentFloor.IsWalkableFloor())
	{
		return HorizontalDelta;
	}

	const FVector FloorNormal = MovementComponent->CurrentFloor.HitResult.ImpactNormal.GetSafeNormal();
	const FVector GroundDelta = FVector::VectorPlaneProject(HorizontalDelta, FloorNormal);
	return GroundDelta.IsNearlyZero()
		? HorizontalDelta
		: GroundDelta.GetSafeNormal() * DeltaDistance;
}

bool DodgeMoveLaunchDelta(
	AMVCharacterBase& OwnerCharacter,
	const FVector& RequestedHorizontalDelta,
	const FVector& LaunchDelta,
	FHitResult& OutSweepHit,
	bool& bOutWalkableHit,
	bool& bOutSteppedUp,
	bool& bOutSlid)
{
	OutSweepHit = FHitResult();
	bOutWalkableHit = false;
	bOutSteppedUp = false;
	bOutSlid = false;

	if (LaunchDelta.IsNearlyZero())
	{
		return true;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter.GetCharacterMovement();
	if (!MovementComponent || !MovementComponent->UpdatedComponent)
	{
		return false;
	}

	UMovementComponent* PublicMovementComponent = MovementComponent;
	const FQuat ComponentRotation = MovementComponent->UpdatedComponent->GetComponentQuat();
	MovementComponent->SafeMoveUpdatedComponent(LaunchDelta, ComponentRotation, true, OutSweepHit);
	if (!OutSweepHit.IsValidBlockingHit())
	{
		MovementComponent->FindFloor(
			MovementComponent->UpdatedComponent->GetComponentLocation(),
			MovementComponent->CurrentFloor,
			false);
		return true;
	}

	bOutWalkableHit = MovementComponent->IsWalkable(OutSweepHit);
	const float RemainingTime = FMath::Clamp(1.0f - OutSweepHit.Time, 0.0f, 1.0f);
	if (OutSweepHit.bStartPenetrating)
	{
		FHitResult SlideHit = OutSweepHit;
		PublicMovementComponent->SlideAlongSurface(
			RequestedHorizontalDelta,
			1.0f,
			SlideHit.Normal,
			SlideHit,
			true);
		bOutSlid = true;
		MovementComponent->FindFloor(
			MovementComponent->UpdatedComponent->GetComponentLocation(),
			MovementComponent->CurrentFloor,
			false);
		return !SlideHit.bStartPenetrating;
	}

	if (RemainingTime > KINDA_SMALL_NUMBER && MovementComponent->CanStepUp(OutSweepHit))
	{
		FStepDownResult StepDownResult;
		if (MovementComponent->StepUp(
			MovementComponent->GetGravityDirection(),
			RequestedHorizontalDelta * RemainingTime,
			OutSweepHit,
			&StepDownResult))
		{
			bOutSteppedUp = true;
			MovementComponent->FindFloor(
				MovementComponent->UpdatedComponent->GetComponentLocation(),
				MovementComponent->CurrentFloor,
				false);
			return true;
		}
	}

	if (RemainingTime <= KINDA_SMALL_NUMBER)
	{
		MovementComponent->FindFloor(
			MovementComponent->UpdatedComponent->GetComponentLocation(),
			MovementComponent->CurrentFloor,
			false);
		return true;
	}

	if (!bOutWalkableHit)
	{
		return false;
	}

	FHitResult SlideHit = OutSweepHit;
	PublicMovementComponent->SlideAlongSurface(
		RequestedHorizontalDelta,
		RemainingTime,
		SlideHit.Normal,
		SlideHit,
		true);
	bOutSlid = true;

	MovementComponent->FindFloor(
		MovementComponent->UpdatedComponent->GetComponentLocation(),
		MovementComponent->CurrentFloor,
		false);
	return !SlideHit.IsValidBlockingHit() || MovementComponent->IsWalkable(SlideHit);
}
}

UMVDodgeComponent::UMVDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVDodgeComponent::BeginPlay()
{
	Super::BeginPlay();

	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	OwnerCharacter->OnMovementInputReceived.AddUObject(this, &UMVDodgeComponent::HandleOwnerMovementInput);

	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		ActionComponent->OnActionPreparing.AddUniqueDynamic(
			this,
			&UMVDodgeComponent::HandleActionPreparing);
		ActionComponent->OnActionStarted.AddUniqueDynamic(
			this,
			&UMVDodgeComponent::HandleActionStarted);
		ActionComponent->ResolveBufferedActionMovementInput.BindUObject(
			this,
			&UMVDodgeComponent::ResolveBufferedActionMovementInput);
		ActionComponent->CanConsumeBufferedAction.BindUObject(
			this,
			&UMVDodgeComponent::CanConsumeBufferedAction);
	}
}

void UMVDodgeComponent::PrepareDodgeAction()
{
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	FVector2D ControllerSpaceMovementInput = FVector2D::ZeroVector;
	bool bUseBufferedSnapshot = false;
	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		bUseBufferedSnapshot = ActionComponent->IsConsumingBufferedAction()
			&& ActionComponent->GetConsumingBufferedActionId() == MVActionIds::Dodge;
		if (bUseBufferedSnapshot && ActionComponent->HasConsumingBufferedActionMovementInput())
		{
			ControllerSpaceMovementInput = DodgeControllerSpaceInputFromVector(
				ActionComponent->GetConsumingBufferedActionMovementInputDirection());
		}
	}

	if (!bUseBufferedSnapshot)
	{
		ControllerSpaceMovementInput = CaptureControllerSpaceMovementInput(*OwnerCharacter);
	}

	const bool bHasMovementInput = !ControllerSpaceMovementInput.IsNearlyZero();
	const bool bFreeDodge = bHasMovementInput
		&& !OwnerCharacter->CharacterInputState.WantsToStrafe
		&& !OwnerCharacter->CharacterInputState.WantsToAim;
	const bool bStrafeDodge = !bFreeDodge
		&& (OwnerCharacter->CharacterInputState.WantsToStrafe || OwnerCharacter->CharacterInputState.WantsToAim);
	const FRotator StrafeReferenceRotation = ResolveStrafeReferenceRotation(*OwnerCharacter);
	const FVector MovementInputDirection = DodgeResolveWorldDirectionFromControllerSpaceInput(
		ControllerSpaceMovementInput,
		StrafeReferenceRotation);

	ELocomotionDirection StrafeInputDirection = ELocomotionDirection::F;
	if (bHasMovementInput && bStrafeDodge)
	{
		StrafeInputDirection = DodgeResolveDirectionFromControllerSpaceInput(ControllerSpaceMovementInput);
	}

	const FVector DodgeMovementDirection = bHasMovementInput && bStrafeDodge
		? ResolveDirectionVectorFromReferenceRotation(StrafeInputDirection, StrafeReferenceRotation)
		: MovementInputDirection;
	const FVector DodgeFacingDirection = ResolveDodgeFacingDirection(
		DodgeMovementDirection,
		bHasMovementInput,
		bFreeDodge,
		StrafeInputDirection);
	if (!DodgeFacingDirection.IsNearlyZero())
	{
		OwnerCharacter->SetActorRotation(MakeYawRotationFromDirection(DodgeFacingDirection));
		if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			MovementComponent->bUseControllerDesiredRotation = false;
			MovementComponent->bOrientRotationToMovement = false;
		}
	}
	ApplyDodgeChooserSnapshot(*OwnerCharacter, bHasMovementInput, bFreeDodge, StrafeInputDirection);

	PreparedDodgeLaunchDirection = bHasMovementInput
		? DodgeMovementDirection.GetSafeNormal2D()
		: -OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	bHasPreparedDodgeLaunchDirection = !PreparedDodgeLaunchDirection.IsNearlyZero();
	bPreparedDodgeHasMovementInput = bHasMovementInput;
	bPreparedDodgeUsesRollDistance = bFreeDodge;
	bPendingBufferedDodgeLaunchFallback = bUseBufferedSnapshot;
	CachedControllerSpaceMovementInput = FVector2D::ZeroVector;
	CachedControllerSpaceMovementInputFrame = 0;
}

void UMVDodgeComponent::UpdateBufferedDodgeMovementInput(const FVector& MovementInputDirection)
{
	HandleOwnerMovementInput(MovementInputDirection);
}

void UMVDodgeComponent::CacheControllerSpaceMovementInput(const FVector2D& ControllerSpaceMovementInput)
{
	CachedControllerSpaceMovementInput = DodgeClampControllerSpaceInput(ControllerSpaceMovementInput);
	CachedControllerSpaceMovementInputFrame = GFrameCounter;
}

void UMVDodgeComponent::HandleOwnerMovementInput(const FVector& MovementInputDirection)
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	UMVActionComponent* ActionComponent = OwnerCharacter
		? OwnerCharacter->ActionComponent
		: nullptr;

	const bool bHasActionBufferContext = ActionComponent
		&& (ActionComponent->HasBufferedAction()
			|| ActionComponent->IsInputBufferOpen()
			|| ActionComponent->IsRecoveryEscapeWindowOpen()
			|| ActionComponent->IsConsumingBufferedAction());
	FVector2D ControllerSpaceMovementInput = FVector2D::ZeroVector;
	if (OwnerCharacter)
	{
		if (!OwnerCharacter->TryGetControllerSpaceMovementInput(ControllerSpaceMovementInput, 0))
		{
			ControllerSpaceMovementInput = DodgeResolveControllerSpaceInputFromWorldDirection(
				MovementInputDirection,
				ResolveStrafeReferenceRotation(*OwnerCharacter));
		}
	}

	if (bHasActionBufferContext && !ControllerSpaceMovementInput.IsNearlyZero())
	{
		CacheControllerSpaceMovementInput(ControllerSpaceMovementInput);
	}

	if (ActionComponent)
	{
		ActionComponent->UpdateBufferedActionMovementInput(
			DodgeControllerSpaceInputToVector(ControllerSpaceMovementInput));
	}
}

bool UMVDodgeComponent::BeginDodgeLaunchWindow(
	const float NotifyDuration,
	UCurveFloat* DistanceCurve,
	const float DistanceScale,
	const bool bApplyVerticalLaunch,
	const int32 MontageInstanceId,
	const bool bClearPreparedLaunch)
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

	CacheDodgeLaunchWindowSettings(
		NotifyDuration,
		DistanceCurve,
		DistanceScale,
		bApplyVerticalLaunch);

	StopActiveDodgeLaunch(false, false);
	ActiveDodgeLaunchMontageInstanceId = MontageInstanceId;

	const FMVActionStatRow* ActionStat = ActionComponent->FindActionStatRow(
		ActionComponent->GetActiveActionId(),
		EMVActionType::Dodge);
	if (!ActionStat)
	{
		ActiveDodgeLaunchMontageInstanceId = INDEX_NONE;
		return false;
	}

	const float LaunchDistance = ResolveDodgeLaunchDistance(*ActionStat, *OwnerCharacter);
	const float LaunchDuration = ResolveDodgeLaunchDuration(*ActionStat, NotifyDuration);
	const float ScaledLaunchDistance = LaunchDistance * FMath::Max(0.0f, DistanceScale);
	const float LaunchVerticalSpeed = ActionStat->LaunchVerticalSpeed;
	if ((ScaledLaunchDistance <= 0.0f || LaunchDuration <= KINDA_SMALL_NUMBER)
		&& (!bApplyVerticalLaunch || FMath::IsNearlyZero(LaunchVerticalSpeed)))
	{
		ActiveDodgeLaunchMontageInstanceId = INDEX_NONE;
		return false;
	}

	const FVector LaunchDirection = ResolveDodgeLaunchDirection(*OwnerCharacter);
	if (bClearPreparedLaunch)
	{
		ClearPreparedDodgeLaunch();
	}
	if (ScaledLaunchDistance > 0.0f && LaunchDuration > KINDA_SMALL_NUMBER && LaunchDirection.IsNearlyZero())
	{
		ActiveDodgeLaunchMontageInstanceId = INDEX_NONE;
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

void UMVDodgeComponent::TickDodgeLaunchWindow(const float DeltaTime, const int32 MontageInstanceId)
{
	if (!IsCurrentDodgeLaunchMontageInstance(MontageInstanceId))
	{
		return;
	}

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
		const FVector HorizontalLaunchDelta = ActiveDodgeLaunchDirection.GetSafeNormal2D() * DeltaDistance;
		const FVector LaunchDelta = ResolveGroundAdjustedLaunchDelta(
			*OwnerCharacter,
			ActiveDodgeLaunchDirection,
			DeltaDistance);
		bool bWalkableHit = false;
		bool bSteppedUp = false;
		bool bSlid = false;
		const bool bLaunchMoveContinued = DodgeMoveLaunchDelta(
			*OwnerCharacter,
			HorizontalLaunchDelta,
			LaunchDelta,
			SweepHit,
			bWalkableHit,
			bSteppedUp,
			bSlid);
		if (!bLaunchMoveContinued)
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

void UMVDodgeComponent::EndDodgeLaunchWindow(
	const bool bClearHorizontalVelocity,
	const int32 MontageInstanceId)
{
	if (!IsCurrentDodgeLaunchMontageInstance(MontageInstanceId))
	{
		return;
	}

	StopActiveDodgeLaunch(bClearHorizontalVelocity);
}

void UMVDodgeComponent::ClearPreparedDodgeLaunch()
{
	PreparedDodgeLaunchDirection = FVector::ZeroVector;
	bHasPreparedDodgeLaunchDirection = false;
	bPreparedDodgeHasMovementInput = false;
	bPreparedDodgeUsesRollDistance = false;
}

void UMVDodgeComponent::StopActiveDodgeLaunch(
	const bool bClearHorizontalVelocity,
	const bool bClearPreparedLaunch)
{
	AMVCharacterBase* OwnerCharacter = ActiveDodgeLaunchCharacter.Get();
	if (bDodgeLaunchActive && bClearHorizontalVelocity && OwnerCharacter)
	{
		OwnerCharacter->LaunchCharacter(FVector::ZeroVector, true, false);
	}

	ActiveDodgeLaunchCharacter = nullptr;
	ActiveDodgeLaunchDistanceCurve = nullptr;
	CachedControllerSpaceMovementInput = FVector2D::ZeroVector;
	CachedControllerSpaceMovementInputFrame = 0;
	ActiveDodgeLaunchDirection = FVector::ZeroVector;
	ActiveDodgeLaunchTargetDistance = 0.0f;
	ActiveDodgeLaunchDuration = 0.0f;
	ActiveDodgeLaunchElapsed = 0.0f;
	ActiveDodgeLaunchLastAlpha = 0.0f;
	ActiveDodgeLaunchMontageInstanceId = INDEX_NONE;
	bDodgeLaunchActive = false;
	if (bClearPreparedLaunch)
	{
		ClearPreparedDodgeLaunch();
	}
}

void UMVDodgeComponent::CacheDodgeLaunchWindowSettings(
	const float NotifyDuration,
	UCurveFloat* DistanceCurve,
	const float DistanceScale,
	const bool bApplyVerticalLaunch)
{
	CachedDodgeLaunchNotifyDuration = NotifyDuration;
	CachedDodgeLaunchDistanceCurve = DistanceCurve;
	CachedDodgeLaunchDistanceScale = DistanceScale;
	bCachedDodgeLaunchApplyVertical = bApplyVerticalLaunch;
	bHasCachedDodgeLaunchWindowSettings = true;
}

bool UMVDodgeComponent::TryStartBufferedDodgeLaunchFallback()
{
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	UMVActionComponent* ActionComponent = OwnerCharacter
		? OwnerCharacter->ActionComponent
		: nullptr;
	if (!OwnerCharacter || !ActionComponent)
	{
		return false;
	}

	const int32 MontageInstanceId = ResolveActiveMontageInstanceId(*OwnerCharacter, *ActionComponent);
	const float NotifyDuration = bHasCachedDodgeLaunchWindowSettings
		? CachedDodgeLaunchNotifyDuration
		: 0.0f;
	UCurveFloat* DistanceCurve = bHasCachedDodgeLaunchWindowSettings
		? CachedDodgeLaunchDistanceCurve.Get()
		: nullptr;
	const float DistanceScale = bHasCachedDodgeLaunchWindowSettings
		? CachedDodgeLaunchDistanceScale
		: 1.0f;
	const bool bApplyVerticalLaunch = bHasCachedDodgeLaunchWindowSettings
		? bCachedDodgeLaunchApplyVertical
		: true;

	return BeginDodgeLaunchWindow(
		NotifyDuration,
		DistanceCurve,
		DistanceScale,
		bApplyVerticalLaunch,
		MontageInstanceId,
		false);
}

FVector2D UMVDodgeComponent::CaptureControllerSpaceMovementInput(const AMVCharacterBase& OwnerCharacter) const
{
	if (CachedControllerSpaceMovementInputFrame == GFrameCounter
		&& !CachedControllerSpaceMovementInput.IsNearlyZero())
	{
		return CachedControllerSpaceMovementInput;
	}

	FVector2D CurrentControllerSpaceInput = FVector2D::ZeroVector;
	if (OwnerCharacter.TryGetControllerSpaceMovementInput(CurrentControllerSpaceInput, 0))
	{
		return DodgeClampControllerSpaceInput(CurrentControllerSpaceInput);
	}

	const FVector PendingInput2D(
		OwnerCharacter.GetPendingMovementInputVector().X,
		OwnerCharacter.GetPendingMovementInputVector().Y,
		0.0f);
	if (!PendingInput2D.IsNearlyZero())
	{
		return DodgeResolveControllerSpaceInputFromWorldDirection(
			PendingInput2D,
			ResolveStrafeReferenceRotation(OwnerCharacter));
	}

	return FVector2D::ZeroVector;
}

FVector UMVDodgeComponent::ResolveBufferedActionMovementInput(const int32 ActionId) const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (ActionId == MVActionIds::Dodge && OwnerCharacter)
	{
		const FVector2D ControllerSpaceMovementInput = CaptureControllerSpaceMovementInput(*OwnerCharacter);
		return DodgeControllerSpaceInputToVector(ControllerSpaceMovementInput);
	}

	return FVector::ZeroVector;
}

bool UMVDodgeComponent::CanConsumeBufferedAction(
	const int32,
	const FVector&,
	const bool) const
{
	return true;
}

void UMVDodgeComponent::ApplyDodgeChooserSnapshot(
	AMVCharacterBase& OwnerCharacter,
	const bool bHasMovementInput,
	const bool bFreeDodge,
	const ELocomotionDirection StrafeInputDirection) const
{
	OwnerCharacter.bHasDodgeMovementInput = bHasMovementInput;

	if (!bHasMovementInput)
	{
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(-OwnerCharacter.GetActorForwardVector());
		return;
	}

	if (bFreeDodge)
	{
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(OwnerCharacter.GetActorForwardVector());
		return;
	}

	switch (StrafeInputDirection)
	{
	case ELocomotionDirection::FR:
	case ELocomotionDirection::FL:
	case ELocomotionDirection::F:
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(OwnerCharacter.GetActorForwardVector());
		break;
	case ELocomotionDirection::R:
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(OwnerCharacter.GetActorRightVector());
		break;
	case ELocomotionDirection::BR:
	case ELocomotionDirection::BL:
	case ELocomotionDirection::B:
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(-OwnerCharacter.GetActorForwardVector());
		break;
	case ELocomotionDirection::L:
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(-OwnerCharacter.GetActorRightVector());
		break;
	default:
		OwnerCharacter.ApplyLocomotionDirectionSnapshot(OwnerCharacter.GetActorForwardVector());
		break;
	}
}

FVector UMVDodgeComponent::ResolveDodgeLaunchDirection(const AMVCharacterBase& OwnerCharacter) const
{
	if (bHasPreparedDodgeLaunchDirection && !PreparedDodgeLaunchDirection.IsNearlyZero())
	{
		return PreparedDodgeLaunchDirection.GetSafeNormal2D();
	}

	const FVector2D ControllerSpaceMovementInput = CaptureControllerSpaceMovementInput(OwnerCharacter);
	const bool bBackstep = bHasPreparedDodgeLaunchDirection
		? !bPreparedDodgeHasMovementInput
		: !OwnerCharacter.bHasDodgeMovementInput;
	FVector LaunchDirection = DodgeResolveWorldDirectionFromControllerSpaceInput(
		ControllerSpaceMovementInput,
		ResolveStrafeReferenceRotation(OwnerCharacter));
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

void UMVDodgeComponent::HandleActionPreparing(const int32 ActionId)
{
	if (ActionId == MVActionIds::Dodge)
	{
		PrepareDodgeAction();
	}
	else
	{
		bPendingBufferedDodgeLaunchFallback = false;
		ClearPreparedDodgeLaunch();
	}
}

void UMVDodgeComponent::HandleActionStarted(const int32 ActionId)
{
	if (ActionId != MVActionIds::Dodge)
	{
		bPendingBufferedDodgeLaunchFallback = false;
		ClearPreparedDodgeLaunch();
		return;
	}

	if (!bPendingBufferedDodgeLaunchFallback)
	{
		return;
	}

	bPendingBufferedDodgeLaunchFallback = false;
	TryStartBufferedDodgeLaunchFallback();
}

float UMVDodgeComponent::ResolveDodgeLaunchDistance(
	const FMVActionStatRow& ActionStat,
	const AMVCharacterBase& OwnerCharacter) const
{
	const float DefaultDistance = FMath::Max(0.0f, ActionStat.LaunchDistance);
	const bool bUsesRollDistance = bHasPreparedDodgeLaunchDirection
		? bPreparedDodgeUsesRollDistance
		: OwnerCharacter.bHasDodgeMovementInput
			&& !OwnerCharacter.CharacterInputState.WantsToStrafe
			&& !OwnerCharacter.CharacterInputState.WantsToAim;
	return bUsesRollDistance
		? DefaultDistance
		: DefaultDistance * NonRollDodgeLaunchDistanceScale;
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

bool UMVDodgeComponent::IsCurrentDodgeLaunchMontageInstance(const int32 MontageInstanceId) const
{
	return ActiveDodgeLaunchMontageInstanceId == INDEX_NONE
		|| MontageInstanceId == INDEX_NONE
		|| ActiveDodgeLaunchMontageInstanceId == MontageInstanceId;
}
