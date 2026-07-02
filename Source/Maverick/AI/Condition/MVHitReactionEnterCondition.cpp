#include "AI/Condition/MVHitReactionEnterCondition.h"

#include "AIController.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVHitReactionComponent.h"
#include "StateTreeExecutionContext.h"

namespace
{
APawn* HitReactionEnterResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
{
	if (BoundOwner)
	{
		return BoundOwner;
	}

	if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		return AIController->GetPawn();
	}

	return Cast<APawn>(Context.GetOwner());
}

bool HitReactionEnterIsStateTreeHandledType(const EMVActionHitReactionType HitReactionType)
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::LargeHit:
	case EMVActionHitReactionType::KnockDown:
	case EMVActionHitReactionType::Airborne:
		return true;
	case EMVActionHitReactionType::None:
	case EMVActionHitReactionType::SmallHit:
	default:
		return false;
	}
}
}

bool FMVHitReactionEnterCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (InstanceData.bRequireHitReactionType
		&& InstanceData.HitData.HitReactionType == EMVActionHitReactionType::None)
	{
		return false;
	}

	if (InstanceData.bRequireStateTreeHandledHitReactionType
		&& !HitReactionEnterIsStateTreeHandledType(InstanceData.HitData.HitReactionType))
	{
		return false;
	}

	APawn* Owner = HitReactionEnterResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return false;
	}

	if (InstanceData.bRequireOwnerAsVictim && InstanceData.HitData.Victim.Get() != Cast<AMVCharacterBase>(Owner))
	{
		return false;
	}

	if (InstanceData.bRequireHitReactionComponent
		&& !Owner->FindComponentByClass<UMVHitReactionComponent>())
	{
		return false;
	}

	return true;
}
