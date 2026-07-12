#include "AI/Task/MVAttackOpportunityTask.h"

#include "StateTreeExecutionContext.h"

namespace
{
bool AttackOpportunityTaskIsCooldownReady(const FMVAttackOpportunityTaskInstanceData& InstanceData)
{
	if (InstanceData.RequiredReadyActionId.IsNone())
	{
		return true;
	}

	return InstanceData.CombatContext.ReadyActionIds.Contains(InstanceData.RequiredReadyActionId);
}

bool AttackOpportunityTaskCanAttack(const FMVAttackOpportunityTaskInstanceData& InstanceData)
{
	const FMVAICombatContext& CombatContext = InstanceData.CombatContext;
	if (!CombatContext.bHasTarget)
	{
		return false;
	}

	if (InstanceData.bFailWhenActionRunning && CombatContext.bActionRunning)
	{
		return false;
	}

	if (InstanceData.bRequireLineOfSight && !CombatContext.bHasLineOfSight)
	{
		return false;
	}

	if (InstanceData.bRequireAttackCadenceReady && !CombatContext.bAttackCadenceReady)
	{
		return false;
	}

	if (!AttackOpportunityTaskIsCooldownReady(InstanceData))
	{
		return false;
	}

	if (CombatContext.DistanceToTarget < InstanceData.MinDistance)
	{
		return false;
	}

	if (InstanceData.MaxDistance > 0.0f && CombatContext.DistanceToTarget > InstanceData.MaxDistance)
	{
		return false;
	}

	return FMath::Abs(CombatContext.AngleToTarget) <= InstanceData.MaxAbsAngle;
}

EStateTreeRunStatus AttackOpportunityTaskEvaluate(FMVAttackOpportunityTaskInstanceData& InstanceData)
{
	InstanceData.bCanAttack = AttackOpportunityTaskCanAttack(InstanceData);
	return InstanceData.bCanAttack
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Running;
}
}

FMVAttackOpportunityTask::FMVAttackOpportunityTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVAttackOpportunityTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	return AttackOpportunityTaskEvaluate(InstanceData);
}

EStateTreeRunStatus FMVAttackOpportunityTask::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	return AttackOpportunityTaskEvaluate(InstanceData);
}
