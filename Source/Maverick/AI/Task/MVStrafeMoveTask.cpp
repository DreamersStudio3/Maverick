#include "MVStrafeMoveTask.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

namespace
{
bool MVStrafeMoveTaskBuildMoveDirection(
	const FVector& PawnLocation,
	const FVector& TargetLocation,
	const float StrafeSign,
	const float DesiredDistance,
	const float DistanceCorrectionRange,
	const float DistanceCorrectionWeight,
	FVector& OutMoveDirection)
{
	FVector FromTargetToPawn = PawnLocation - TargetLocation;
	FromTargetToPawn.Z = 0.0f;

	const float CurrentDistance = FromTargetToPawn.Size2D();
	if (CurrentDistance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector RadialDir = FromTargetToPawn.GetSafeNormal2D();
	const FVector TangentDir = FVector::CrossProduct(FVector::UpVector, RadialDir).GetSafeNormal2D() * StrafeSign;

	FVector DistanceCorrection = FVector::ZeroVector;
	const float DistanceError = CurrentDistance - DesiredDistance;

	if (DistanceCorrectionRange > KINDA_SMALL_NUMBER && !FMath::IsNearlyZero(DistanceError))
	{
		const float CorrectionAlpha = FMath::Clamp(FMath::Abs(DistanceError) / DistanceCorrectionRange, 0.0f, 1.0f);
		const float CorrectionSign = DistanceError > 0.0f ? -1.0f : 1.0f;
		DistanceCorrection = RadialDir * CorrectionSign * CorrectionAlpha * FMath::Max(0.0f, DistanceCorrectionWeight);
	}

	OutMoveDirection = (TangentDir + DistanceCorrection).GetSafeNormal2D();
	return !OutMoveDirection.IsNearlyZero();
}

bool MVStrafeMoveTaskIsMoveDirectionBlocked(const APawn& Pawn, const FVector& MoveDirection, const float TraceDistance)
{
	if (TraceDistance <= 0.0f)
	{
		return false;
	}

	UWorld* World = Pawn.GetWorld();
	if (!World)
	{
		return true;
	}

	const FVector TraceDirection = MoveDirection.GetSafeNormal2D();
	if (TraceDirection.IsNearlyZero())
	{
		return true;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MVStrafeMoveTask), false);
	QueryParams.AddIgnoredActor(&Pawn);

	const FVector Start = Pawn.GetActorLocation();
	const FVector End = Start + TraceDirection * TraceDistance;

	return World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
}

UCharacterMovementComponent* MVStrafeMoveTaskGetMovementComponent(APawn& Pawn)
{
	ACharacter* Character = Cast<ACharacter>(&Pawn);
	return Character ? Character->GetCharacterMovement() : nullptr;
}

void MVStrafeMoveTaskApplySpeed(FMVStrafeMoveTaskInstanceData& InstanceData, APawn& Pawn)
{
	UCharacterMovementComponent* MovementComponent = MVStrafeMoveTaskGetMovementComponent(Pawn);
	if (!MovementComponent)
	{
		return;
	}

	if (!InstanceData.bAppliedStrafeMoveSpeed)
	{
		InstanceData.PreviousMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
		InstanceData.bAppliedStrafeMoveSpeed = true;
	}

	MovementComponent->MaxWalkSpeed = FMath::Max(0.0f, InstanceData.StrafeMoveSpeed);
}

void MVStrafeMoveTaskRestoreSpeed(FMVStrafeMoveTaskInstanceData& InstanceData, APawn& Pawn)
{
	UCharacterMovementComponent* MovementComponent = MVStrafeMoveTaskGetMovementComponent(Pawn);
	if (!MovementComponent || !InstanceData.bAppliedStrafeMoveSpeed)
	{
		return;
	}

	MovementComponent->MaxWalkSpeed = InstanceData.PreviousMaxWalkSpeed;
	InstanceData.PreviousMaxWalkSpeed = 0.0f;
	InstanceData.bAppliedStrafeMoveSpeed = false;
}
}

FMVStrafeMoveTask::FMVStrafeMoveTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVStrafeMoveTask::EnterState(FStateTreeExecutionContext& Context,
                                                  const FStateTreeTransitionResult& Transition) const
{
	InstanceDataType& InstanceData = Context.GetInstanceData<InstanceDataType>(*this);

	const AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}
	

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}
	UWorld* World = Controller->GetWorld();
	
	
	const AActor* TargetActor = UGameplayStatics::GetPlayerPawn(World,0);
	if (!TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ElapsedTime = 0.0f;
	InstanceData.bStrafePathClear = false;
	InstanceData.PreviousMaxWalkSpeed = 0.0f;
	InstanceData.bAppliedStrafeMoveSpeed = false;
	const float MaxDuration = FMath::Max(InstanceData.StrafeMinDuration, InstanceData.StrafeMaxDuration);
	InstanceData.MoveDuration = MaxDuration;

	FVector MoveDirection = FVector::ZeroVector;
	if (MVStrafeMoveTaskBuildMoveDirection(
		Pawn->GetActorLocation(),
		TargetActor->GetActorLocation(),
		1.0f,
		InstanceData.DesiredDistance,
		InstanceData.DistanceCorrectionRange,
		InstanceData.DistanceCorrectionWeight,
		MoveDirection)
		&& !MVStrafeMoveTaskIsMoveDirectionBlocked(*Pawn, MoveDirection, InstanceData.TraceDistance))
	{
		InstanceData.StrafeSign = 1.0f;
		InstanceData.bStrafePathClear = true;
	}
	else if (MVStrafeMoveTaskBuildMoveDirection(
		Pawn->GetActorLocation(),
		TargetActor->GetActorLocation(),
		-1.0f,
		InstanceData.DesiredDistance,
		InstanceData.DistanceCorrectionRange,
		InstanceData.DistanceCorrectionWeight,
		MoveDirection)
		&& !MVStrafeMoveTaskIsMoveDirectionBlocked(*Pawn, MoveDirection, InstanceData.TraceDistance))
	{
		InstanceData.StrafeSign = -1.0f;
		InstanceData.bStrafePathClear = true;
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}

	MVStrafeMoveTaskApplySpeed(InstanceData, *Pawn);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVStrafeMoveTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	InstanceDataType& InstanceData = Context.GetInstanceData<InstanceDataType>(*this);

	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	
	UWorld* World = Controller->GetWorld();
	
	const AActor* TargetActor = UGameplayStatics::GetPlayerPawn(World,0);
	
	if (!TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector MoveDirection = FVector::ZeroVector;
	if (!MVStrafeMoveTaskBuildMoveDirection(
		Pawn->GetActorLocation(),
		TargetActor->GetActorLocation(),
		InstanceData.StrafeSign,
		InstanceData.DesiredDistance,
		InstanceData.DistanceCorrectionRange,
		InstanceData.DistanceCorrectionWeight,
		MoveDirection))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (MVStrafeMoveTaskIsMoveDirectionBlocked(*Pawn, MoveDirection, InstanceData.TraceDistance))
	{
		InstanceData.bStrafePathClear = false;
		return InstanceData.ElapsedTime >= InstanceData.StrafeMinDuration
			? EStateTreeRunStatus::Succeeded
			: EStateTreeRunStatus::Failed;
	}

	InstanceData.bStrafePathClear = true;
	MVStrafeMoveTaskApplySpeed(InstanceData, *Pawn);
	Pawn->AddMovementInput(MoveDirection, 1.0f);

	InstanceData.ElapsedTime += DeltaTime;
	if (InstanceData.ElapsedTime < InstanceData.StrafeMinDuration)
	{
		return EStateTreeRunStatus::Running;
	}

	const FMVAICombatContext& CombatContext = InstanceData.CombatContext;
	if (CombatContext.bHasTarget && CombatContext.DistanceToTarget > InstanceData.MoveToTargetDistance)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (CombatContext.bCounterWindow
		|| CombatContext.bSprintPathClear
		|| CombatContext.bAirborneChargePathClear
		|| (CombatContext.bAttackCadenceReady && !CombatContext.bNeedAttackAngle && !CombatContext.bNeedClearAttackPath))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.ElapsedTime >= InstanceData.MoveDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FMVStrafeMoveTask::ExitState(FStateTreeExecutionContext& Context,
                                  const FStateTreeTransitionResult& Transition) const
{
	InstanceDataType& InstanceData = Context.GetInstanceData<InstanceDataType>(*this);
	if (const AAIController* Controller = Cast<AAIController>(Context.GetOwner()))
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			MVStrafeMoveTaskRestoreSpeed(InstanceData, *Pawn);
		}
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
