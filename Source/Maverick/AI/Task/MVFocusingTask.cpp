#include "MVFocusingTask.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

namespace
{
AAIController* MVFocusingTaskResolveAIController(
	FStateTreeExecutionContext& Context,
	const APawn* Owner)
{
	if (AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		return AIController;
	}

	return Owner ? Cast<AAIController>(Owner->GetController()) : nullptr;
}
}

EStateTreeRunStatus FMVFocusingTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
	) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.FocusController = nullptr;
	InstanceData.bFocusApplied = false;

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

	AAIController* AIController = MVFocusingTaskResolveAIController(Context, InstanceData.Owner);
	if (!InstanceData.Target || !AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	AIController->SetFocus(InstanceData.Target, EAIFocusPriority::Gameplay);
	InstanceData.FocusController = AIController;
	InstanceData.bFocusApplied = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVFocusingTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.bCanFocus || !InstanceData.Target || !InstanceData.Owner || !InstanceData.FocusController)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FMVFocusingTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
	) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.bFocusApplied)
	{
		if (AAIController* AIController = InstanceData.FocusController.Get())
		{
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
		}
	}

	InstanceData.FocusController = nullptr;
	InstanceData.bFocusApplied = false;

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
