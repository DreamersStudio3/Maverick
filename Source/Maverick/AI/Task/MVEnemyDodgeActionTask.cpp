#include "AI/Task/MVEnemyDodgeActionTask.h"

#include "AIController.h"
#include "Components/MVActionComponent.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"
#include "Tables/MVActionRowTableTypes.h"

namespace
{
APawn* EnemyDodgeActionTaskResolveOwner(
	FStateTreeExecutionContext& Context,
	const TObjectPtr<APawn>& BoundOwner)
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

EMVActionInputDirection EnemyDodgeActionTaskResolveDirection(
	const FMVAIDodgeRequest& DodgeRequest,
	const EMVActionInputDirection DefaultDirection)
{
	return DodgeRequest.Direction == EMVActionInputDirection::None
		? DefaultDirection
		: DodgeRequest.Direction;
}

const FDataTableRowHandle& EnemyDodgeActionTaskSelectRowHandle(
	const FMVEnemyDodgeActionTaskInstanceData& InstanceData)
{
	switch (InstanceData.ResolvedDirection)
	{
	case EMVActionInputDirection::Forward:
		return InstanceData.ForwardDodgeActionRow;
	case EMVActionInputDirection::Left:
		return InstanceData.LeftDodgeActionRow;
	case EMVActionInputDirection::Right:
		return InstanceData.RightDodgeActionRow;
	case EMVActionInputDirection::Back:
	case EMVActionInputDirection::None:
	default:
		return InstanceData.BackDodgeActionRow;
	}
}

const FMVActionRow* EnemyDodgeActionTaskFindActionRow(const FDataTableRowHandle& RowHandle)
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return nullptr;
	}

	if (!RowHandle.DataTable->GetRowStruct()
		|| !RowHandle.DataTable->GetRowStruct()->IsChildOf(FMVActionRow::StaticStruct()))
	{
		return nullptr;
	}

	return RowHandle.DataTable->FindRow<FMVActionRow>(
		RowHandle.RowName,
		TEXT("MVEnemyDodgeActionTask"),
		false);
}

bool EnemyDodgeActionTaskIsStartedActionRunning(
	const FMVEnemyDodgeActionTaskInstanceData& InstanceData)
{
	return InstanceData.ActionComponent
		&& InstanceData.ActionComponent->IsActionRunning()
		&& (InstanceData.StartedActionTableName.IsNone()
			|| InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName)
		&& (InstanceData.StartedActionRowName.IsNone()
			|| InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName);
}
}

FMVEnemyDodgeActionTask::FMVEnemyDodgeActionTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVEnemyDodgeActionTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.bStarted = false;
	InstanceData.ActionComponent = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;
	InstanceData.ResolvedDirection = EnemyDodgeActionTaskResolveDirection(
		InstanceData.DodgeRequest,
		InstanceData.DefaultDirection);

	APawn* Owner = EnemyDodgeActionTaskResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Owner = Owner;
	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FDataTableRowHandle& RowHandle = EnemyDodgeActionTaskSelectRowHandle(InstanceData);
	const FMVActionRow* ActionRow = EnemyDodgeActionTaskFindActionRow(RowHandle);
	if (!ActionRow || !ActionRow->bEnabled)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FName StartSection = InstanceData.StartSection.IsNone()
		? ActionRow->DefaultStartSection
		: InstanceData.StartSection;
	const bool bActionRunning = InstanceData.ActionComponent->IsActionRunning();
	const bool bStarted = bActionRunning && InstanceData.bTransitionFromCurrentAction
		? InstanceData.ActionComponent->TryTransitionActionFromRowHandle(
			RowHandle,
			StartSection,
			InstanceData.TransitionBlendOutTime)
		: InstanceData.ActionComponent->TryStartActionFromRowHandle(RowHandle, StartSection);
	if (!bStarted)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bStarted = true;
	InstanceData.StartedActionTableName = InstanceData.ActionComponent->GetActiveActionTableName();
	InstanceData.StartedActionRowName = InstanceData.ActionComponent->GetActiveActionRowName();
	return InstanceData.bWaitForActionEnd ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FMVEnemyDodgeActionTask::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.bWaitForActionEnd)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EnemyDodgeActionTaskIsStartedActionRunning(InstanceData)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVEnemyDodgeActionTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.bStarted
		&& InstanceData.bCancelActionOnExit
		&& EnemyDodgeActionTaskIsStartedActionRunning(InstanceData))
	{
		InstanceData.ActionComponent->CancelActiveAction(InstanceData.ExitCancelBlendOutTime);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
