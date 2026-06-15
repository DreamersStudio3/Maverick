#include "MVBossLookAtTargetTask.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

EStateTreeRunStatus FMVBossLookAtTargetTask::EnterState(FStateTreeExecutionContext& Context,
                                                        const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.bCanLookAt)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		InstanceData.Owner = AIController->GetPawn();
	}
	else
	{
		InstanceData.Owner = Cast<APawn>(Context.GetOwner());
	}

	if (!InstanceData.Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	UWorld* World = InstanceData.Owner->GetWorld();
	if (!World)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Target = UGameplayStatics::GetPlayerPawn(World, 0);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVBossLookAtTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const FVector TargetLocation = InstanceData.Target->GetActorLocation();
	const FVector OwnerLocation = InstanceData.Owner->GetActorLocation();

	FVector TargetDirection = TargetLocation - OwnerLocation;
	TargetDirection.Z = 0.0f;

	if (TargetDirection.IsNearlyZero())
	{
		return EStateTreeRunStatus::Failed;
	}

	const FRotator TargetRotation = TargetDirection.Rotation();
	const FRotator CurrentRotation = InstanceData.Owner->GetActorRotation();

	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		InstanceData.TurnSpeed);

	InstanceData.Owner->SetActorRotation(NewRotation);

	return EStateTreeRunStatus::Running;
}

void FMVBossLookAtTargetTask::ExitState(FStateTreeExecutionContext& Context,
                                        const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
