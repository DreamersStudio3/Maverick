#include "AI/Condition/MVCombatDecisionCondition.h"

#include "StateTreeExecutionContext.h"

namespace
{
bool CombatDecisionIsCandidateReady(
	const FMVAICombatContext& CombatContext,
	const FMVAICombatActionCondition& Candidate)
{
	if (Candidate.ActionRequest.Domain != EMVActionDomain::Attack
		|| Candidate.ActionRequest.RowName.IsNone())
	{
		return false;
	}

	if (!MVAICombat::IsDistanceInRange(CombatContext, Candidate.MinDistance, Candidate.MaxDistance))
	{
		return false;
	}

	if (FMath::Abs(CombatContext.AngleToTarget) > Candidate.MaxAbsAngle)
	{
		return false;
	}

	if (!MVAICombat::IsActionReady(
		CombatContext,
		MVAICombat::MakeCooldownActionId(Candidate.Metadata, Candidate.ActionRequest)))
	{
		return false;
	}

	if (Candidate.bRequiresLineOfSight && !CombatContext.bHasLineOfSight)
	{
		return false;
	}

	if (Candidate.bRequiresForwardPathClear && !CombatContext.bSprintPathClear)
	{
		return false;
	}

	if (Candidate.bRequiresBackwardPathClear && !CombatContext.bStrafePathClear)
	{
		return false;
	}

	if (Candidate.bRequiresTargetCanBeAirborne && !CombatContext.bTargetCanBeAirborne)
	{
		return false;
	}

	if (Candidate.bRequiresTargetNotAirborne && CombatContext.bTargetIsAirborne)
	{
		return false;
	}

	if (Candidate.bRequiresAirbornePattern && !CombatContext.bShouldUseAirborneCharge)
	{
		return false;
	}

	return true;
}

bool CombatDecisionHasReadyCandidate(
	const FMVAICombatContext& CombatContext,
	const TArray<FMVAICombatActionCondition>& Candidates)
{
	for (const FMVAICombatActionCondition& Candidate : Candidates)
	{
		if (CombatDecisionIsCandidateReady(CombatContext, Candidate))
		{
			return true;
		}
	}

	return false;
}

bool CombatDecisionCanAttackImmediately(const FMVCombatDecisionConditionInstanceData& InstanceData)
{
	const FMVAICombatContext& CombatContext = InstanceData.CombatContext;
	return CombatDecisionIsCandidateReady(CombatContext, InstanceData.CounterAttack)
		|| CombatDecisionIsCandidateReady(CombatContext, InstanceData.SprintAttack)
		|| CombatDecisionIsCandidateReady(CombatContext, InstanceData.AirborneChargeAttack)
		|| CombatDecisionHasReadyCandidate(CombatContext, InstanceData.SkillAttacks)
		|| CombatDecisionHasReadyCandidate(CombatContext, InstanceData.BasicAttacks);
}
}

bool FMVCombatDecisionCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	const FMVAICombatContext& CombatContext = InstanceData.CombatContext;

	if (InstanceData.DesiredState == EMVAICombatDecisionState::Dead)
	{
		return CombatContext.bIsDead;
	}

	if (CombatContext.bIsDead || !CombatContext.bHasTarget || CombatContext.bActionRunning)
	{
		return false;
	}

	switch (InstanceData.DesiredState)
	{
	case EMVAICombatDecisionState::CounterAttack:
		return CombatContext.bCounterWindow
			&& CombatDecisionIsCandidateReady(CombatContext, InstanceData.CounterAttack);

	case EMVAICombatDecisionState::SprintAttack:
		return CombatDecisionIsCandidateReady(CombatContext, InstanceData.SprintAttack);

	case EMVAICombatDecisionState::AirborneChargeAttack:
		return CombatDecisionIsCandidateReady(CombatContext, InstanceData.AirborneChargeAttack);

	case EMVAICombatDecisionState::SkillAttack:
		return CombatContext.bAttackCadenceReady
			&& CombatDecisionHasReadyCandidate(CombatContext, InstanceData.SkillAttacks);

	case EMVAICombatDecisionState::BasicAttack:
		return CombatContext.bAttackCadenceReady
			&& CombatDecisionHasReadyCandidate(CombatContext, InstanceData.BasicAttacks);

	case EMVAICombatDecisionState::MoveToTarget:
		return CombatContext.DistanceToTarget > InstanceData.SprintMaxReach
			|| CombatContext.DistanceToTarget > InstanceData.CombatMaxDistance;

	case EMVAICombatDecisionState::Strafe:
		return CombatContext.DistanceToTarget <= InstanceData.CombatMaxDistance
			&& !CombatDecisionCanAttackImmediately(InstanceData)
			&& (CombatContext.bStrafePathClear
				|| !CombatContext.bAttackCadenceReady
				|| CombatContext.bNeedAttackAngle
				|| CombatContext.bNeedClearAttackPath);

	case EMVAICombatDecisionState::Idle:
		return true;

	default:
		return false;
	}
}
