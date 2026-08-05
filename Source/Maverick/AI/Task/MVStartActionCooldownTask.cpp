#include "MVStartActionCooldownTask.h"

#include "AI/MVActionCooldownComponent.h"
#include "AIController.h"
#include "Components/MVActionComponent.h"
#include "StateTreeExecutionContext.h"

namespace
{
	APawn* ResolveStartCooldownOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

EStateTreeRunStatus FMVStartActionCooldownTask::EnterState(FStateTreeExecutionContext& Context,
                                                           const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	const APawn* Owner = ResolveStartCooldownOwner(Context, InstanceData.Owner);
	if (!Owner || InstanceData.ActionId.IsNone())
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bRequireStartedAction)
	{
		const UMVActionComponent* ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
		const bool bHasMatchingAction = ActionComponent
			&& ActionComponent->IsActionRunning()
			&& (InstanceData.StartedActionTableName.IsNone()
				|| ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName)
			&& (InstanceData.StartedActionRowName.IsNone()
				|| ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName);
		if (!bHasMatchingAction)
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	UMVActionCooldownComponent* CooldownComponent = Owner->FindComponentByClass<UMVActionCooldownComponent>();
	if (!CooldownComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	return CooldownComponent->StartCooldown(InstanceData.ActionId)
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Failed;
}
