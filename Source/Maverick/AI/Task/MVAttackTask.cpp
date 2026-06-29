#include "MVAttackTask.h"

#include "Character/Npc/Enemy/MVEnemy.h"
#include "AIController.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FMVAttackTask::EnterState(FStateTreeExecutionContext& Context,
                                              const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Enemy && InstanceData.AttackMontageEndedHandle.IsValid())
	{
		InstanceData.Enemy->OnAttackMontageEnded.Remove(InstanceData.AttackMontageEndedHandle);
	}

	InstanceData.Pawn = nullptr;
	InstanceData.Enemy = nullptr;
	InstanceData.AttackInstanceId = INDEX_NONE;
	InstanceData.AttackMontageEndedHandle.Reset();

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

	InstanceData.Enemy = Cast<AMVEnemy>(InstanceData.Pawn);
	if (!InstanceData.Enemy)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.Enemy->Attack(InstanceData.AttackDirection, InstanceData.AttackInstanceId))
	{
		return EStateTreeRunStatus::Failed;
	}

	const int32 ExpectedAttackInstanceId = InstanceData.AttackInstanceId;
	InstanceData.AttackMontageEndedHandle = InstanceData.Enemy->OnAttackMontageEnded.AddLambda(
		[WeakContext = Context.MakeWeakExecutionContext(), ExpectedAttackInstanceId](
			const int32 FinishedAttackInstanceId,
			UAnimMontage* Montage,
			const bool bInterrupted)
		{
			if (FinishedAttackInstanceId == ExpectedAttackInstanceId)
			{
				WeakContext.FinishTask(bInterrupted
					? EStateTreeFinishTaskType::Failed
					: EStateTreeFinishTaskType::Succeeded);
			}
		});

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return EStateTreeRunStatus::Running;
}

void FMVAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Enemy && InstanceData.AttackMontageEndedHandle.IsValid())
	{
		InstanceData.Enemy->OnAttackMontageEnded.Remove(InstanceData.AttackMontageEndedHandle);
		InstanceData.AttackMontageEndedHandle.Reset();
	}

	FStateTreeTaskBase::ExitState(Context, Transition);
}
