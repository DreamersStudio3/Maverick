#include "AI/Condition/MVGroggyEnterCondition.h"

#include "AIController.h"
#include "Components/MVStatComponent.h"
#include "StateTreeExecutionContext.h"

namespace
{
APawn* GroggyEnterResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

bool FMVGroggyEnterCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	const APawn* Owner = GroggyEnterResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return false;
	}

	const UMVStatComponent* StatComponent = Owner->FindComponentByClass<UMVStatComponent>();
	return StatComponent ? StatComponent->IsGroggy() : false;
}
