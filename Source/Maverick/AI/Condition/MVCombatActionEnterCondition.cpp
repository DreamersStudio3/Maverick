#include "AI/Condition/MVCombatActionEnterCondition.h"

#include "StateTreeExecutionContext.h"

namespace
{
bool CombatContextEnterPassesCooldownGroupCheck(const FMVCombatActionEnterConditionInstanceData& InstanceData)
{
	if (InstanceData.CooldownCheckMode == EMVCombatCooldownCheckMode::None)
	{
		return true;
	}

	int32 ValidActionIdCount = 0;
	int32 ReadyActionIdCount = 0;
	for (const FName ActionId : InstanceData.CooldownActionIds)
	{
		if (ActionId.IsNone())
		{
			continue;
		}

		++ValidActionIdCount;
		if (MVAICombat::IsActionReady(InstanceData.CombatContext, ActionId))
		{
			++ReadyActionIdCount;
		}
	}

	if (ValidActionIdCount <= 0)
	{
		return false;
	}

	switch (InstanceData.CooldownCheckMode)
	{
	case EMVCombatCooldownCheckMode::AnyReady:
		return ReadyActionIdCount > 0;
	case EMVCombatCooldownCheckMode::AllReady:
		return ReadyActionIdCount == ValidActionIdCount;
	case EMVCombatCooldownCheckMode::NoneReady:
		return ReadyActionIdCount == 0;
	case EMVCombatCooldownCheckMode::None:
	default:
		return true;
	}
}

bool CombatContextEnterPassesCommonGate(const FMVCombatActionEnterConditionInstanceData& InstanceData)
{
	const FMVAICombatContext& CombatContext = InstanceData.CombatContext;
	if (CombatContext.bIsDead)
	{
		return false;
	}

	if (!CombatContext.bHasTarget)
	{
		return false;
	}

	if (InstanceData.bRequireCombatArea && CombatContext.CurrentArea != InstanceData.RequiredCombatArea)
	{
		return false;
	}

	if (!CombatContextEnterPassesCooldownGroupCheck(InstanceData))
	{
		return false;
	}

	return true;
}

bool CombatContextEnterIsDistanceInRange(const FMVCombatActionEnterConditionInstanceData& InstanceData)
{
	return MVAICombat::IsDistanceInRange(
		InstanceData.CombatContext,
		InstanceData.MinDistance,
		InstanceData.MaxDistance);
}

bool CombatContextEnterCanEnterAction(const FMVCombatActionEnterConditionInstanceData& InstanceData)
{
	const FMVAICombatContext& CombatContext = InstanceData.CombatContext;
	if (InstanceData.ActionType == EMVCombatActionTypes::None)
	{
		return false;
	}

	if (!CombatContextEnterPassesCommonGate(InstanceData) || CombatContext.bActionRunning)
	{
		return false;
	}

	if (!CombatContextEnterIsDistanceInRange(InstanceData))
	{
		return false;
	}

	if (FMath::Abs(CombatContext.AngleToTarget) > InstanceData.MaxAbsAngle)
	{
		return false;
	}

	return InstanceData.CooldownActionId.IsNone()
		|| MVAICombat::IsActionReady(CombatContext, InstanceData.CooldownActionId);
}

bool CombatContextEnterCanMoveToTarget(const FMVCombatActionEnterConditionInstanceData& InstanceData)
{
	if (!CombatContextEnterPassesCommonGate(InstanceData) || InstanceData.CombatContext.bActionRunning)
	{
		return false;
	}

	return InstanceData.MaxDistance > 0.0f
		&& InstanceData.CombatContext.DistanceToTarget > InstanceData.MaxDistance;
}

bool CombatContextEnterCanStrafe(const FMVCombatActionEnterConditionInstanceData& InstanceData)
{
	const FMVAICombatContext& CombatContext = InstanceData.CombatContext;
	if (!CombatContextEnterPassesCommonGate(InstanceData) || CombatContext.bActionRunning)
	{
		return false;
	}

	return CombatContextEnterIsDistanceInRange(InstanceData)
		&& CombatContext.bStrafePathClear;
}
}

bool FMVCombatActionEnterCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	switch (InstanceData.Mode)
	{
	case EMVCombatContextEnterMode::Dead:
		return InstanceData.CombatContext.bIsDead;

	case EMVCombatContextEnterMode::Action:
		return CombatContextEnterCanEnterAction(InstanceData);

	case EMVCombatContextEnterMode::MoveToTarget:
		return CombatContextEnterCanMoveToTarget(InstanceData);

	case EMVCombatContextEnterMode::Strafe:
		return CombatContextEnterCanStrafe(InstanceData);

	case EMVCombatContextEnterMode::Idle:
		return !InstanceData.CombatContext.bIsDead;

	default:
		return false;
	}
}
