#include "MVFocusingTask.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

EStateTreeRunStatus FMVFocusingTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
	) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.bCanFocus)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.Owner)
	{
		if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
		{
			InstanceData.Owner = AIController->GetPawn();
		}
		else
		{
			InstanceData.Owner = Cast<APawn>(Context.GetOwner());
		}
	}

	if (!InstanceData.Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.Target)
	{
		if (UWorld* World = InstanceData.Owner->GetWorld())
		{
			InstanceData.Target = UGameplayStatics::GetPlayerPawn(World, 0);
		}
	}

	return InstanceData.Target ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FMVFocusingTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.bCanFocus || !InstanceData.Target || !InstanceData.Owner)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	FVector TargetDirection = InstanceData.Target->GetActorLocation() - InstanceData.Owner->GetActorLocation();
	TargetDirection.Z = 0.0f;
	
	if (TargetDirection.IsNearlyZero())
	{
		return EStateTreeRunStatus::Failed;
	}
	
	FRotator TargetRotation = TargetDirection.Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;
	
	const FRotator NewRotation = FMath::RInterpTo(
		InstanceData.Owner->GetActorRotation(),
		TargetRotation,
		DeltaTime,
		InstanceData.TurnSpeed);
	
	InstanceData.Owner->SetActorRotation(NewRotation);

	return EStateTreeRunStatus::Running;
}

void FMVFocusingTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
	) const
{
	
	
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
