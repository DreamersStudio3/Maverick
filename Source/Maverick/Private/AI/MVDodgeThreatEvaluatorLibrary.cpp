#include "AI/MVDodgeThreatEvaluatorLibrary.h"

#include "Tags/MVGameplayTags.h"

namespace
{
bool MVDodgeThreatIsActionTypeAllowed(
	const EMVCombatActionTypes ActionType,
	const TArray<EMVCombatActionTypes>& ThreatActionTypes)
{
	if (ThreatActionTypes.IsEmpty())
	{
		return ActionType == EMVCombatActionTypes::HeavyAttack
			|| ActionType == EMVCombatActionTypes::ChargeAttack
			|| ActionType == EMVCombatActionTypes::Skill
			|| ActionType == EMVCombatActionTypes::SprintHeavyAttack;
	}

	return ThreatActionTypes.Contains(ActionType);
}

EMVActionInputDirection MVDodgeThreatResolveEscapeDirection(
	const AActor& Owner,
	const FVector& DirectionToThreat)
{
	if (DirectionToThreat.IsNearlyZero())
	{
		return EMVActionInputDirection::Back;
	}

	const FVector ThreatDirection = DirectionToThreat.GetSafeNormal2D();
	const FVector Forward = Owner.GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = Owner.GetActorRightVector().GetSafeNormal2D();
	const float ForwardDot = FVector::DotProduct(ThreatDirection, Forward);
	const float RightDot = FVector::DotProduct(ThreatDirection, Right);

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f
			? EMVActionInputDirection::Back
			: EMVActionInputDirection::Forward;
	}

	return RightDot >= 0.0f
		? EMVActionInputDirection::Left
		: EMVActionInputDirection::Right;
}

float MVDodgeThreatResolveSignedAngle(
	const AActor& Owner,
	const FVector& DirectionToThreat)
{
	if (DirectionToThreat.IsNearlyZero())
	{
		return 0.0f;
	}

	const FVector ThreatDirection = DirectionToThreat.GetSafeNormal2D();
	const FVector Forward = Owner.GetActorForwardVector().GetSafeNormal2D();
	const float Dot = FVector::DotProduct(Forward, ThreatDirection);
	const float CrossZ = FVector::CrossProduct(Forward, ThreatDirection).Z;
	return FMath::RadiansToDegrees(FMath::Atan2(CrossZ, Dot));
}

FGameplayTag MVDodgeThreatResolveActionTag(const FMVCombatActionEvent& CombatActionEvent)
{
	if (CombatActionEvent.ActionType == EMVCombatActionTypes::Skill
		&& CombatActionEvent.ActionIndex == 0)
	{
		return MVGameplayTags::Action_Combat_Skill_Q;
	}

	switch (CombatActionEvent.ActionType)
	{
	case EMVCombatActionTypes::LightAttack:
	case EMVCombatActionTypes::HeavyAttack:
	case EMVCombatActionTypes::ChargeAttack:
	case EMVCombatActionTypes::Skill:
	case EMVCombatActionTypes::SprintLightAttack:
	case EMVCombatActionTypes::SprintHeavyAttack:
	case EMVCombatActionTypes::DodgeLightAttack:
	case EMVCombatActionTypes::DodgeHeavyAttack:
		return MVGameplayTags::Action_Combat;
	default:
		return FGameplayTag();
	}
}
}

FMVDodgeThreatConfig::FMVDodgeThreatConfig()
{
	ThreatActionTypes =
	{
		EMVCombatActionTypes::HeavyAttack,
		EMVCombatActionTypes::ChargeAttack,
		EMVCombatActionTypes::Skill,
		EMVCombatActionTypes::SprintHeavyAttack
	};
}

FMVDodgeThreatDecision UMVDodgeThreatEvaluatorLibrary::EvaluateDodgeThreat(
	AActor* Owner,
	AActor* Target,
	const FMVCombatActionEvent& CombatActionEvent,
	const FMVDodgeThreatConfig& Config,
	const bool bOwnerActionRunning)
{
	FMVDodgeThreatDecision Decision;

	if (!Config.bEnabled)
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::Disabled;
		return Decision;
	}

	if (!IsValid(Owner))
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::InvalidOwner;
		return Decision;
	}

	AActor* ThreatActor = CombatActionEvent.Instigator;
	if (!IsValid(ThreatActor))
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::InvalidThreat;
		return Decision;
	}

	if (ThreatActor == Owner)
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::ThreatIsSelf;
		return Decision;
	}

	if (Config.bRequireInstigatorIsTarget && ThreatActor != Target)
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::TargetMismatch;
		return Decision;
	}

	if (!MVDodgeThreatIsActionTypeAllowed(CombatActionEvent.ActionType, Config.ThreatActionTypes))
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::ActionTypeRejected;
		return Decision;
	}

	if (bOwnerActionRunning && !Config.bAllowWhileActionRunning)
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::ActionRunning;
		return Decision;
	}

	const FVector DirectionToThreat = ThreatActor->GetActorLocation() - Owner->GetActorLocation();
	Decision.DistanceToThreat = DirectionToThreat.Size2D();
	Decision.AngleToThreat = MVDodgeThreatResolveSignedAngle(*Owner, DirectionToThreat);

	if (Config.MaxDistance > 0.0f && Decision.DistanceToThreat > Config.MaxDistance)
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::TooFar;
		return Decision;
	}

	if (FMath::Abs(Decision.AngleToThreat) > Config.MaxAbsAngle)
	{
		Decision.RejectReason = EMVDodgeThreatRejectReason::OutsideAngle;
		return Decision;
	}

	Decision.bShouldSendEvent = true;
	Decision.RejectReason = EMVDodgeThreatRejectReason::None;
	Decision.DodgeRequest.Direction = MVDodgeThreatResolveEscapeDirection(*Owner, DirectionToThreat);
	Decision.DodgeRequest.ThreatActor = ThreatActor;
	Decision.DodgeRequest.ThreatLocation = ThreatActor->GetActorLocation();
	Decision.DodgeRequest.ThreatActionType = MVDodgeThreatResolveActionTag(CombatActionEvent);
	Decision.DodgeRequest.DistanceToThreat = Decision.DistanceToThreat;
	Decision.DodgeRequest.AngleToThreat = Decision.AngleToThreat;

	return Decision;
}
