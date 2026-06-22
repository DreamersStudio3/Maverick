#include "Components/MVDodgeComponent.h"

#include "Character/MVCharacterBase.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/MVActionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tables/MVActionTableTypes.h"

namespace
{
constexpr float NonRollDodgeLaunchDistanceScale = 0.75f;

const TCHAR* DodgeDebugBoolText(const bool bValue)
{
	return bValue ? TEXT("true") : TEXT("false");
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

FVector GetReferenceForwardVector(const FRotator& ReferenceRotation)
{
	return FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::X).GetSafeNormal2D();
}

FVector GetReferenceRightVector(const FRotator& ReferenceRotation)
{
	return FRotationMatrix(ReferenceRotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
}

ELocomotionDirection ResolveDirectionFromReferenceRotation(
	const FVector& WorldDirection,
	const FRotator& ReferenceRotation)
{
	const FVector Direction2D = NormalizeMovementInputDirection(WorldDirection);
	if (Direction2D.IsNearlyZero())
	{
		return ELocomotionDirection::F;
	}

	const float ForwardDot = FVector::DotProduct(Direction2D, GetReferenceForwardVector(ReferenceRotation));
	const float RightDot = FVector::DotProduct(Direction2D, GetReferenceRightVector(ReferenceRotation));
	const float DirectionAngle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
	return ResolveDodgeEightWayDirection(DirectionAngle);
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

FRotator ResolveStrafeReferenceRotation(const AMVCharacterBase& OwnerCharacter)
{
	if (const AController* Controller = OwnerCharacter.GetController())
	{
		return FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	}

	return FRotator(0.0f, OwnerCharacter.GetActorRotation().Yaw, 0.0f);
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

	FVector MovementInputDirection = FVector::ZeroVector;
	bool bUseBufferedSnapshot = false;
	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		bUseBufferedSnapshot = ActionComponent->IsConsumingBufferedAction()
			&& ActionComponent->GetConsumingBufferedActionId() == MVActionIds::Dodge;
		if (bUseBufferedSnapshot && ActionComponent->HasConsumingBufferedActionMovementInput())
		{
			MovementInputDirection = ActionComponent->GetConsumingBufferedActionMovementInputDirection();
		}
	}

	if (!bUseBufferedSnapshot)
	{
		MovementInputDirection = CaptureMovementInputDirection(*OwnerCharacter);
	}
	const bool bHasMovementInput = !MovementInputDirection.IsNearlyZero();
	const bool bFreeDodge = bHasMovementInput
		&& !OwnerCharacter->CharacterInputState.WantsToStrafe
		&& !OwnerCharacter->CharacterInputState.WantsToAim;
	const bool bStrafeDodge = !bFreeDodge
		&& (OwnerCharacter->CharacterInputState.WantsToStrafe || OwnerCharacter->CharacterInputState.WantsToAim);

	const FRotator StrafeReferenceRotation = ResolveStrafeReferenceRotation(*OwnerCharacter);
	if (bStrafeDodge)
	{
		OwnerCharacter->SetActorRotation(StrafeReferenceRotation);
	}

	ELocomotionDirection StrafeInputDirection = ELocomotionDirection::F;
	if (bHasMovementInput && bStrafeDodge)
	{
		StrafeInputDirection = ResolveDirectionFromReferenceRotation(
			MovementInputDirection,
			StrafeReferenceRotation);
	}

	const FVector DodgeMovementDirection = bHasMovementInput && bStrafeDodge
		? ResolveDirectionVectorFromReferenceRotation(StrafeInputDirection, StrafeReferenceRotation)
		: MovementInputDirection;
	ApplyDodgeChooserSnapshot(*OwnerCharacter, bHasMovementInput, bFreeDodge, StrafeInputDirection);

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

	PreparedDodgeLaunchDirection = bHasMovementInput
		? DodgeMovementDirection.GetSafeNormal2D()
		: -OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	bHasPreparedDodgeLaunchDirection = !PreparedDodgeLaunchDirection.IsNearlyZero();
	bPreparedDodgeHasMovementInput = bHasMovementInput;
	bPreparedDodgeUsesRollDistance = bFreeDodge;
	bPendingBufferedDodgeLaunchFallback = bUseBufferedSnapshot;
	CachedMovementInputDirection = FVector::ZeroVector;
}

void UMVDodgeComponent::UpdateBufferedDodgeMovementInput(const FVector& MovementInputDirection)
{
	HandleOwnerMovementInput(MovementInputDirection);
}

void UMVDodgeComponent::CacheMovementInputDirection(const FVector& MovementInputDirection)
{
	CachedMovementInputDirection = NormalizeMovementInputDirection(MovementInputDirection);
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
	if (bHasActionBufferContext && !MovementInputDirection.IsNearlyZero())
	{
		CacheMovementInputDirection(MovementInputDirection);
	}

	if (ActionComponent)
	{
		ActionComponent->UpdateBufferedActionMovementInput(MovementInputDirection);
	}
}

bool UMVDodgeComponent::BeginDodgeLaunchWindow(
	const float NotifyDuration,
	UCurveFloat* DistanceCurve,
	const float DistanceScale,
	const bool bApplyVerticalLaunch,
	const int32 MontageInstanceId)
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
	ClearPreparedDodgeLaunch();
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

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVDodgeLaunch] Begin Owner=%s MontageInstance=%d Direction=%s Distance=%.2f Duration=%.3f Vertical=%.2f Active=%s"),
		*GetNameSafe(OwnerCharacter),
		MontageInstanceId,
		*LaunchDirection.ToCompactString(),
		ScaledLaunchDistance,
		LaunchDuration,
		bApplyVerticalLaunch ? LaunchVerticalSpeed : 0.0f,
		DodgeDebugBoolText(bDodgeLaunchActive));

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
		const FVector LaunchDelta = ResolveGroundAdjustedLaunchDelta(
			*OwnerCharacter,
			ActiveDodgeLaunchDirection,
			DeltaDistance);
		OwnerCharacter->AddActorWorldOffset(LaunchDelta, true, &SweepHit);
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

void UMVDodgeComponent::EndDodgeLaunchWindow(
	const bool bClearHorizontalVelocity,
	const int32 MontageInstanceId)
{
	if (!IsCurrentDodgeLaunchMontageInstance(MontageInstanceId))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVDodgeLaunch] IgnoreStaleEnd Owner=%s EndMontageInstance=%d ActiveMontageInstance=%d Active=%s"),
			*GetNameSafe(GetOwner()),
			MontageInstanceId,
			ActiveDodgeLaunchMontageInstanceId,
			DodgeDebugBoolText(bDodgeLaunchActive));
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVDodgeLaunch] End Owner=%s MontageInstance=%d ClearHorizontal=%s Active=%s Elapsed=%.3f Duration=%.3f"),
		*GetNameSafe(GetOwner()),
		MontageInstanceId,
		DodgeDebugBoolText(bClearHorizontalVelocity),
		DodgeDebugBoolText(bDodgeLaunchActive),
		ActiveDodgeLaunchElapsed,
		ActiveDodgeLaunchDuration);
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
	CachedMovementInputDirection = FVector::ZeroVector;
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

	const bool bStarted = BeginDodgeLaunchWindow(
		NotifyDuration,
		DistanceCurve,
		DistanceScale,
		bApplyVerticalLaunch,
		MontageInstanceId);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVDodgeLaunch] BufferedFallback Owner=%s Started=%s MontageInstance=%d HasCachedSettings=%s"),
		*GetNameSafe(OwnerCharacter),
		DodgeDebugBoolText(bStarted),
		MontageInstanceId,
		DodgeDebugBoolText(bHasCachedDodgeLaunchWindowSettings));
	return bStarted;
}

FVector UMVDodgeComponent::CaptureMovementInputDirection(const AMVCharacterBase& OwnerCharacter) const
{
	if (!CachedMovementInputDirection.IsNearlyZero())
	{
		return CachedMovementInputDirection;
	}

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
	if (!PendingInput2D.IsNearlyZero())
	{
		return PendingInput2D.GetSafeNormal2D();
	}

	const FVector LastInput2D(
		OwnerCharacter.GetLastMovementInputVector().X,
		OwnerCharacter.GetLastMovementInputVector().Y,
		0.0f);
	return LastInput2D.GetSafeNormal2D();
}

FVector UMVDodgeComponent::ResolveBufferedActionMovementInput(const int32 ActionId) const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (ActionId == MVActionIds::Dodge && OwnerCharacter)
	{
		const FVector MovementInputDirection = CaptureMovementInputDirection(*OwnerCharacter);
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] DodgeResolveBufferedInput Owner=%s Action=%d Dir=%s HasInput=%s"),
			*GetNameSafe(OwnerCharacter),
			ActionId,
			*MovementInputDirection.ToCompactString(),
			DodgeDebugBoolText(!MovementInputDirection.IsNearlyZero()));
		return MovementInputDirection;
	}

	return FVector::ZeroVector;
}

bool UMVDodgeComponent::CanConsumeBufferedAction(
	const int32 ActionId,
	const FVector& MovementInputDirection,
	const bool bHasMovementInput) const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter
		? OwnerCharacter->ActionComponent
		: nullptr;
	if (ActionId != MVActionIds::Dodge || !ActionComponent)
	{
		return true;
	}

	const bool bActiveActionIsDodge =
		ActionComponent->FindActionIndexRow(ActionComponent->GetActiveActionId(), EMVActionType::Dodge) != nullptr;
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[MVActionBuffer] DodgeCanConsumeBufferedAction Owner=%s Action=%d Active=%d ActiveIsDodge=%s HasInput=%s Dir=%s"),
		*GetNameSafe(OwnerCharacter),
		ActionId,
		ActionComponent->GetActiveActionId(),
		DodgeDebugBoolText(bActiveActionIsDodge),
		DodgeDebugBoolText(bHasMovementInput),
		*MovementInputDirection.ToCompactString());
	if (bActiveActionIsDodge && !bHasMovementInput)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MVActionBuffer] DodgeConsumeRejected NoMovementInputDuringDodge Owner=%s Action=%d Active=%d"),
			*GetNameSafe(OwnerCharacter),
			ActionId,
			ActionComponent->GetActiveActionId());
		return false;
	}

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

	const FVector Acceleration2D = CaptureMovementInputDirection(OwnerCharacter);
	const bool bBackstep = bHasPreparedDodgeLaunchDirection
		? !bPreparedDodgeHasMovementInput
		: !OwnerCharacter.bHasDodgeMovementInput;
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

void UMVDodgeComponent::HandleActionPreparing(const int32 ActionId)
{
	if (ActionId == MVActionIds::Dodge)
	{
		PrepareDodgeAction();
	}
	else
	{
		bPendingBufferedDodgeLaunchFallback = false;
	}
}

void UMVDodgeComponent::HandleActionStarted(const int32 ActionId)
{
	if (ActionId != MVActionIds::Dodge)
	{
		bPendingBufferedDodgeLaunchFallback = false;
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
