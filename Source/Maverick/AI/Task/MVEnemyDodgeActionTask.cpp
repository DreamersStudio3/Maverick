#include "AI/Task/MVEnemyDodgeActionTask.h"

#include "AIController.h"
#include "Chooser.h"
#include "Components/MVActionComponent.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVEnemyDodgeActionTask, Log, All);

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

bool EnemyDodgeActionTaskEvaluateChooser(
	UObject& OwnerObject,
	FMVEnemyDodgeActionTaskInstanceData& InstanceData,
	FMVDodgeActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	InstanceData.ChooserDodgeActionRow.Reset();

	if (!InstanceData.DodgeChooserTable.IsValid())
	{
		return false;
	}

	UChooserTable* ChooserTable = Cast<UChooserTable>(InstanceData.DodgeChooserTable.TryLoad());
	if (!ChooserTable)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Enemy dodge chooser failed to load. Path=%s."),
			*InstanceData.DodgeChooserTable.ToString());
		return false;
	}

	FMVDodgeActionRowHandle ChooserDodgeActionRow;
	ChooserDodgeActionRow.Reset();
	FMVAIDodgeRequest DodgeRequest = InstanceData.DodgeRequest;
	DodgeRequest.Direction = InstanceData.ResolvedDirection;
	FGameplayTag ThreatActionType = DodgeRequest.ThreatActionType;

	FChooserEvaluationContext ChooserContext;
	ChooserContext.AddObjectParam(&OwnerObject);
	ChooserContext.AddStructParam(DodgeRequest);
	ChooserContext.AddStructParam(ChooserDodgeActionRow);
	ChooserContext.AddStructParam(ThreatActionType);

	TSoftObjectPtr<UObject> SelectedObject;
	UChooserTable::EvaluateChooser(
		ChooserContext,
		ChooserTable,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));

	if (ChooserDodgeActionRow.IsValid())
	{
		OutActionRowHandle = ChooserDodgeActionRow;
		if (OutActionRowHandle.ActionRow.RowName.IsNone())
		{
			const FDataTableRowHandle& DirectionRowHandle = EnemyDodgeActionTaskSelectRowHandle(InstanceData);
			OutActionRowHandle.ActionRow.RowName = InstanceData.FallbackDodgeActionRow.ActionRow.RowName.IsNone()
				? DirectionRowHandle.RowName
				: InstanceData.FallbackDodgeActionRow.ActionRow.RowName;
		}
		InstanceData.ChooserDodgeActionRow = ChooserDodgeActionRow;
		return OutActionRowHandle.IsValid();
	}

	UObject* ResolvedObject = SelectedObject.IsValid()
		? SelectedObject.Get()
		: SelectedObject.LoadSynchronous();
	UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
	if (!SelectedDataTable)
	{
		return false;
	}

	const FDataTableRowHandle& DirectionRowHandle = EnemyDodgeActionTaskSelectRowHandle(InstanceData);
	OutActionRowHandle.ActionRow.DataTable = SelectedDataTable;
	OutActionRowHandle.ActionRow.RowName = InstanceData.FallbackDodgeActionRow.ActionRow.RowName.IsNone()
		? DirectionRowHandle.RowName
		: InstanceData.FallbackDodgeActionRow.ActionRow.RowName;
	OutActionRowHandle.StartSection = InstanceData.FallbackDodgeActionRow.StartSection;
	return OutActionRowHandle.IsValid();
}

FMVDodgeActionRowHandle EnemyDodgeActionTaskResolveActionRowHandle(
	UObject& OwnerObject,
	FMVEnemyDodgeActionTaskInstanceData& InstanceData)
{
	FMVDodgeActionRowHandle ResolvedRowHandle;
	ResolvedRowHandle.Reset();

	if (EnemyDodgeActionTaskEvaluateChooser(OwnerObject, InstanceData, ResolvedRowHandle))
	{
		return ResolvedRowHandle;
	}

	if (InstanceData.FallbackDodgeActionRow.IsValid())
	{
		return InstanceData.FallbackDodgeActionRow;
	}

	ResolvedRowHandle.ActionRow = EnemyDodgeActionTaskSelectRowHandle(InstanceData);
	return ResolvedRowHandle;
}

const FMVDodgeActionRow* EnemyDodgeActionTaskFindActionRow(const FDataTableRowHandle& RowHandle)
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return nullptr;
	}

	if (!RowHandle.DataTable->GetRowStruct()
		|| !RowHandle.DataTable->GetRowStruct()->IsChildOf(FMVDodgeActionRow::StaticStruct()))
	{
		return nullptr;
	}

	return RowHandle.DataTable->FindRow<FMVDodgeActionRow>(
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
	InstanceData.ResolvedDodgeActionRow.Reset();
	InstanceData.ChooserDodgeActionRow.Reset();
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

	InstanceData.ResolvedDodgeActionRow = EnemyDodgeActionTaskResolveActionRowHandle(*Owner, InstanceData);
	const FDataTableRowHandle& RowHandle = InstanceData.ResolvedDodgeActionRow.ActionRow;
	const FMVDodgeActionRow* ActionRow = EnemyDodgeActionTaskFindActionRow(RowHandle);
	if (!ActionRow || !ActionRow->bEnabled)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FName RequestedStartSection = InstanceData.StartSection.IsNone()
		? InstanceData.ResolvedDodgeActionRow.StartSection
		: InstanceData.StartSection;
	const FName StartSection = RequestedStartSection.IsNone()
		? ActionRow->DefaultStartSection
		: RequestedStartSection;
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
