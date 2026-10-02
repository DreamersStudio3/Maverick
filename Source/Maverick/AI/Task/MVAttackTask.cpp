#include "MVAttackTask.h"

#include "AIController.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Enum/MVCombatActionTypes.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FMVAttackTask::EnterState(FStateTreeExecutionContext& Context,
                                              const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.Pawn = nullptr;
	InstanceData.CombatComponent = nullptr;
	InstanceData.ActionComponent = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;

	if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		InstanceData.Pawn = AIController->GetPawn();
	}
	else
	{
		InstanceData.Pawn = Cast<APawn>(Context.GetOwner());
	}

	if (!InstanceData.Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CombatComponent = InstanceData.Pawn->FindComponentByClass<UMVCombatComponent>();
	InstanceData.ActionComponent = InstanceData.Pawn->FindComponentByClass<UMVActionComponent>();
	if (!InstanceData.CombatComponent || !InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.CombatComponent->TryCombatAction(EMVCombatActionTypes::LightAttack))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.StartedActionTableName = InstanceData.ActionComponent->GetActiveActionTableName();
	InstanceData.StartedActionRowName = InstanceData.ActionComponent->GetActiveActionRowName();

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	const bool bActionRunning = InstanceData.ActionComponent->IsActionRunning()
		&& (InstanceData.StartedActionTableName.IsNone()
			|| InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName)
		&& (InstanceData.StartedActionRowName.IsNone()
			|| InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName);

	return bActionRunning ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

void FMVAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskBase::ExitState(Context, Transition);
}
