#include "AI/Task/MVHitReactionTask.h"

#include "AIController.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVActionComponent.h"
#include "Components/MVHitReactionComponent.h"
#include "StateTreeExecutionContext.h"

namespace
{
APawn* HitReactionTaskResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

bool HitReactionTaskIsStartedActionRunning(const FMVHitReactionTaskInstanceData& InstanceData)
{
	if (!InstanceData.ActionComponent || !InstanceData.ActionComponent->IsActionRunning())
	{
		return false;
	}

	if (InstanceData.StartedActionRowName.IsNone())
	{
		return true;
	}

	return InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName
		&& InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName;
}

bool HitReactionTaskIsStateTreeHandledType(const EMVActionHitReactionType HitReactionType)
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::Stagger:
	case EMVActionHitReactionType::Knockback:
	case EMVActionHitReactionType::KnockDown:
	case EMVActionHitReactionType::Airborne:
	case EMVActionHitReactionType::Groggy:
		return true;
	case EMVActionHitReactionType::None:
	case EMVActionHitReactionType::Flinch:
	default:
		return false;
	}
}

}

FMVHitReactionTask::FMVHitReactionTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVHitReactionTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ActionComponent = nullptr;
	InstanceData.HitReactionComponent = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;

	if (InstanceData.HitData.HitReactionType == EMVActionHitReactionType::None)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bRequireStateTreeHandledHitReactionType
		&& !HitReactionTaskIsStateTreeHandledType(InstanceData.HitData.HitReactionType))
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Owner = HitReactionTaskResolveOwner(Context, InstanceData.Owner);
	if (!Owner || InstanceData.HitData.Victim.Get() != Cast<AMVCharacterBase>(Owner))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.HitReactionComponent = Owner->FindComponentByClass<UMVHitReactionComponent>();
	if (!InstanceData.HitReactionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();

	InstanceData.HitReactionComponent->HandleDamaged(InstanceData.HitData);

	if (InstanceData.ActionComponent && InstanceData.ActionComponent->IsActionRunning())
	{
		InstanceData.StartedActionTableName = InstanceData.ActionComponent->GetActiveActionTableName();
		InstanceData.StartedActionRowName = InstanceData.ActionComponent->GetActiveActionRowName();
		return InstanceData.bWaitForActionEnd
			? EStateTreeRunStatus::Running
			: EStateTreeRunStatus::Succeeded;
	}

	return InstanceData.bFailWhenReactionNotStarted
		? EStateTreeRunStatus::Failed
		: EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FMVHitReactionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	return HitReactionTaskIsStartedActionRunning(InstanceData)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVHitReactionTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.bCancelReactionOnExit
		&& InstanceData.bWaitForActionEnd
		&& HitReactionTaskIsStartedActionRunning(InstanceData))
	{
		InstanceData.ActionComponent->CancelActiveAction(InstanceData.ExitCancelBlendOutTime);
	}

	if (InstanceData.bClearHitDataOnExit)
	{
		InstanceData.HitData = FMVResolvedHitData();
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
