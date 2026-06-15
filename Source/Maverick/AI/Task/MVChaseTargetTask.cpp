#include "MVChaseTargetTask.h"

#include "AIController.h"
#include "HeadMountedDisplayTypes.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

EStateTreeRunStatus FMVChaseTargetTask::EnterState(FStateTreeExecutionContext& Context,
                                                   const FStateTreeTransitionResult& Transition) const
{
	InstanceDataType& InstanceData = Context.GetInstanceData<InstanceDataType>(*this);
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
	if (!InstanceData.Target)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVChaseTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return FStateTreeTaskCommonBase::Tick(Context, DeltaTime);
}

void FMVChaseTargetTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}