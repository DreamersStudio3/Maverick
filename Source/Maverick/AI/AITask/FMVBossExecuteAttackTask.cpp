#include "AI/AITask/FMVBossExecuteAttackTask.h"

#include "AIController.h"
#include "Components/MVActionComponent.h"
#include "StateTreeExecutionContext.h"

FMVBossExecuteAttackTask::FMVBossExecuteAttackTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVBossExecuteAttackTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ActionComponent = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;

	APawn* Owner = InstanceData.Owner;
	if (!Owner)
	{
		if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
		{
			Owner = AIController->GetPawn();
		}
		else
		{
			Owner = Cast<APawn>(Context.GetOwner());
		}
	}

	if (!Owner
		|| !InstanceData.CombatContext.bHasTarget
		|| InstanceData.CombatContext.DistanceToTarget > InstanceData.AttackRange
		|| !InstanceData.AttackRow.DataTable
		|| InstanceData.AttackRow.RowName.IsNone())
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (!InstanceData.ActionComponent
		|| !InstanceData.ActionComponent->TryStartActionFromRowHandle(
			InstanceData.AttackRow,
			InstanceData.StartSection))
	{
		return EStateTreeRunStatus::Failed;
	}

	FString ActionTableName = InstanceData.AttackRow.DataTable->GetName();
	ActionTableName.RemoveFromStart(TEXT("DT_"));
	InstanceData.StartedActionTableName = FName(*ActionTableName);
	InstanceData.StartedActionRowName = InstanceData.AttackRow.RowName;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVBossExecuteAttackTask::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	const bool bStartedActionRunning = InstanceData.ActionComponent->IsActionRunning()
		&& InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName
		&& InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName;

	return bStartedActionRunning
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVBossExecuteAttackTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.ActionComponent
		&& InstanceData.ActionComponent->IsActionRunning()
		&& InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName
		&& InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName)
	{
		InstanceData.ActionComponent->CancelActiveAction(0.1f);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
