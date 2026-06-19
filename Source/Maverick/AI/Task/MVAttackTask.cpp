#include "MVAttackTask.h"

#include "AI/MVEnemy.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FMVAttackTask::EnterState(FStateTreeExecutionContext& Context,
                                              const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		InstanceData.Pawn = AIController->GetPawn();
	}
	else
	{
		InstanceData.Pawn = Cast<APawn>(Context.GetOwner());
	}

	if (!InstanceData.Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	AMVEnemy* Enemy = Cast<AMVEnemy>(InstanceData.Pawn);
	if (!Enemy)
	{
		return EStateTreeRunStatus::Failed;
	}

	return Enemy->Attack(InstanceData.AttackDirection)
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FMVAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return FStateTreeTaskBase::Tick(Context, DeltaTime);
}

void FMVAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskBase::ExitState(Context, Transition);
}
