#include "MVActionCooldownReadyCondition.h"

#include "AI/MVActionCooldownComponent.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

namespace
{
	APawn* ResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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
}

bool FMVActionCooldownReadyCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	const APawn* Owner = ResolveOwner(Context, InstanceData.Owner);
	if (!Owner || InstanceData.ActionId.IsNone())
	{
		return false;
	}

	const UMVActionCooldownComponent* CooldownComponent = Owner->FindComponentByClass<UMVActionCooldownComponent>();
	return CooldownComponent ? CooldownComponent->IsCooldownReady(InstanceData.ActionId) : false;
}
