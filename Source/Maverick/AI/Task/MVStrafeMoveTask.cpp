#include "MVStrafeMoveTask.h"

#include "AIController.h"
#include "Engine/World.h"
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
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}

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
		InstanceData.StrafeSign *= -1.0f;
		if (!MVStrafeMoveTaskBuildMoveDirection(
			Pawn->GetActorLocation(),
			TargetActor->GetActorLocation(),
			InstanceData.StrafeSign,
			InstanceData.DesiredDistance,
			InstanceData.DistanceCorrectionRange,
			InstanceData.DistanceCorrectionWeight,
			MoveDirection)
			|| MVStrafeMoveTaskIsMoveDirectionBlocked(*Pawn, MoveDirection, InstanceData.TraceDistance))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	Pawn->AddMovementInput(MoveDirection, 1.0f);

	InstanceData.ElapsedTime += DeltaTime;
	if (InstanceData.ElapsedTime >= InstanceData.MoveDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FMVStrafeMoveTask::ExitState(FStateTreeExecutionContext& Context,
                                  const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
