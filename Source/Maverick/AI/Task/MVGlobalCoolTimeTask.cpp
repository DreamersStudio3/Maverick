#include "MVGlobalCoolTimeTask.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"

namespace
{
	APawn* ResolveCooldownOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

	UMVActionCooldownComponent* EnsureCooldownComponent(APawn& Owner)
	{
		if (UMVActionCooldownComponent* ExistingComponent = Owner.FindComponentByClass<UMVActionCooldownComponent>())
		{
			return ExistingComponent;
		}

		UMVActionCooldownComponent* NewComponent = NewObject<UMVActionCooldownComponent>(&Owner, TEXT("ActionCooldownComponent"));
		if (!NewComponent)
		{
			return nullptr;
		}

		Owner.AddInstanceComponent(NewComponent);
		NewComponent->RegisterComponent();
		return NewComponent;
	}

	void UpdateQueryOutputs(FMVGlobalCoolTimeTaskInstanceData& InstanceData)
	{
		if (!InstanceData.CooldownComponent || InstanceData.QueryActionId.IsNone())
		{
			InstanceData.bIsQueryActionReady = false;
			InstanceData.QueryActionRemainingTime = 0.0f;
			return;
		}

		InstanceData.bIsQueryActionReady = InstanceData.CooldownComponent->IsCooldownReady(InstanceData.QueryActionId);
		InstanceData.QueryActionRemainingTime = InstanceData.CooldownComponent->GetRemainingTime(InstanceData.QueryActionId);
	}
}

FMVGlobalCoolTimeTask::FMVGlobalCoolTimeTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVGlobalCoolTimeTask::EnterState(FStateTreeExecutionContext& Context,
                                                      const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	InstanceData.Owner = ResolveCooldownOwner(Context, InstanceData.Owner);
	if (!InstanceData.Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CooldownComponent = EnsureCooldownComponent(*InstanceData.Owner);
	if (!InstanceData.CooldownComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CooldownComponent->ConfigureCooldowns(InstanceData.ActionCooldowns);
	UpdateQueryOutputs(InstanceData);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVGlobalCoolTimeTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.CooldownComponent)
	{
		InstanceData.Owner = ResolveCooldownOwner(Context, InstanceData.Owner);
		if (!InstanceData.Owner)
		{
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.CooldownComponent = EnsureCooldownComponent(*InstanceData.Owner);
		if (!InstanceData.CooldownComponent)
		{
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.CooldownComponent->ConfigureCooldowns(InstanceData.ActionCooldowns);
	}

	InstanceData.CooldownComponent->TickCooldowns(DeltaTime);
	UpdateQueryOutputs(InstanceData);

	return EStateTreeRunStatus::Running;
}

void FMVGlobalCoolTimeTask::ExitState(FStateTreeExecutionContext& Context,
                                      const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
