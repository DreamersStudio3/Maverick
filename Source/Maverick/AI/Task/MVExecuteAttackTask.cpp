#include "AI/Task/MVExecuteAttackTask.h"

#include "AI/MVActionCooldownComponent.h"
#include "AI/MVEnemy.h"
#include "AIController.h"
#include "Components/MVActionComponent.h"
#include "Engine/DataTable.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"

namespace
{
APawn* ExecuteAttackResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

FName ExecuteAttackActionTableNameFromDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

bool ExecuteAttackTryStartAction(
	UMVActionComponent& ActionComponent,
	const FMVAICombatActionCandidate& Candidate,
	FName& OutActionTableName,
	FName& OutActionRowName)
{
	OutActionTableName = ExecuteAttackActionTableNameFromDataTable(Candidate.ActionRow.DataTable);
	OutActionRowName = Candidate.ActionRow.RowName;
	if (!MVAICombat::HasExecutableActionRow(Candidate))
	{
		return false;
	}

	return ActionComponent.TryStartActionFromRowHandle(Candidate.ActionRow, Candidate.StartSection);
}

bool ExecuteAttackIsStartedActionRunning(
	const UMVActionComponent& ActionComponent,
	const FName StartedActionTableName,
	const FName StartedActionRowName)
{
	return ActionComponent.IsActionRunning()
		&& ActionComponent.GetActiveActionTableName() == StartedActionTableName
		&& ActionComponent.GetActiveActionRowName() == StartedActionRowName;
}

bool ExecuteAttackStartCooldown(APawn& Owner, const FMVAICombatActionCandidate& Candidate)
{
	UMVActionCooldownComponent* CooldownComponent = Owner.FindComponentByClass<UMVActionCooldownComponent>();
	if (!CooldownComponent)
	{
		return true;
	}

	const FName CooldownActionId = MVAICombat::MakeCooldownActionId(Candidate);
	return CooldownActionId.IsNone() || CooldownComponent->StartCooldown(CooldownActionId);
}

bool ExecuteAttackCanSelectCandidate(
	const FMVAICombatContext& CombatContext,
	const FMVAICombatActionCandidate& Candidate,
	const bool bAllowRepeatedLastAttack)
{
	if (!MVAICombat::HasExecutableActionRow(Candidate))
	{
		return false;
	}

	if (!MVAICombat::IsDistanceInRange(CombatContext, Candidate.MinDistance, Candidate.MaxDistance))
	{
		return false;
	}

	if (FMath::Abs(CombatContext.AngleToTarget) > Candidate.MaxAbsAngle)
	{
		return false;
	}

	if (!MVAICombat::IsActionReady(CombatContext, MVAICombat::MakeCooldownActionId(Candidate)))
	{
		return false;
	}

	if (!bAllowRepeatedLastAttack && MVAICombat::MakeActionTag(Candidate) == CombatContext.LastAttackTag)
	{
		return false;
	}

	if (Candidate.bRequiresLineOfSight && !CombatContext.bHasLineOfSight)
	{
		return false;
	}

	if (Candidate.bRequiresForwardPathClear && !CombatContext.bSprintPathClear)
	{
		return false;
	}

	if (Candidate.bRequiresBackwardPathClear && !CombatContext.bStrafePathClear)
	{
		return false;
	}

	if (Candidate.bRequiresTargetCanBeAirborne && !CombatContext.bTargetCanBeAirborne)
	{
		return false;
	}

	if (Candidate.bRequiresTargetNotAirborne && CombatContext.bTargetIsAirborne)
	{
		return false;
	}

	if (Candidate.bRequiresAirbornePattern && !CombatContext.bShouldUseAirborneCharge)
	{
		return false;
	}

	return true;
}

bool ExecuteAttackSelectCandidate(
	const FMVAICombatContext& CombatContext,
	const TArray<FMVAICombatActionCandidate>& Candidates,
	FMVAICombatActionCandidate& OutCandidate)
{
	for (const FMVAICombatActionCandidate& Candidate : Candidates)
	{
		if (ExecuteAttackCanSelectCandidate(CombatContext, Candidate, false))
		{
			OutCandidate = Candidate;
			return true;
		}
	}

	for (const FMVAICombatActionCandidate& Candidate : Candidates)
	{
		if (ExecuteAttackCanSelectCandidate(CombatContext, Candidate, true))
		{
			OutCandidate = Candidate;
			return true;
		}
	}

	return false;
}
}

FMVExecuteFixedAttackTask::FMVExecuteFixedAttackTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVExecuteFixedAttackTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Enemy && InstanceData.AttackMontageEndedHandle.IsValid())
	{
		InstanceData.Enemy->OnAttackMontageEnded.Remove(InstanceData.AttackMontageEndedHandle);
	}

	InstanceData.ActionComponent = nullptr;
	InstanceData.Enemy = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;
	InstanceData.AttackInstanceId = INDEX_NONE;
	InstanceData.AttackMontageEndedHandle.Reset();

	APawn* Owner = ExecuteAttackResolveOwner(Context, InstanceData.Owner);
	if (!Owner || !MVAICombat::HasExecutableActionRow(InstanceData.Attack))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (InstanceData.ActionComponent)
	{
		if (!ExecuteAttackTryStartAction(
			*InstanceData.ActionComponent,
			InstanceData.Attack,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
		{
			return EStateTreeRunStatus::Failed;
		}

		if (!ExecuteAttackStartCooldown(*Owner, InstanceData.Attack))
		{
			InstanceData.ActionComponent->CancelActiveAction(0.0f);
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.LastAttackTag = MVAICombat::MakeActionTag(InstanceData.Attack);
		return EStateTreeRunStatus::Running;
	}

	InstanceData.Enemy = Cast<AMVEnemy>(Owner);
	if (!InstanceData.Enemy || !InstanceData.Enemy->Attack(InstanceData.FallbackAttackDirection, InstanceData.AttackInstanceId))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!ExecuteAttackStartCooldown(*Owner, InstanceData.Attack))
	{
		return EStateTreeRunStatus::Failed;
	}

	const int32 ExpectedAttackInstanceId = InstanceData.AttackInstanceId;
	InstanceData.AttackMontageEndedHandle = InstanceData.Enemy->OnAttackMontageEnded.AddLambda(
		[WeakContext = Context.MakeWeakExecutionContext(), ExpectedAttackInstanceId](
			const int32 FinishedAttackInstanceId,
			UAnimMontage* Montage,
			const bool bInterrupted)
		{
			if (FinishedAttackInstanceId == ExpectedAttackInstanceId)
			{
				WeakContext.FinishTask(bInterrupted
					? EStateTreeFinishTaskType::Failed
					: EStateTreeFinishTaskType::Succeeded);
			}
		});

	InstanceData.LastAttackTag = MVAICombat::MakeActionTag(InstanceData.Attack);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVExecuteFixedAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Running;
	}

	return ExecuteAttackIsStartedActionRunning(
		*InstanceData.ActionComponent,
		InstanceData.StartedActionTableName,
		InstanceData.StartedActionRowName)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVExecuteFixedAttackTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Enemy && InstanceData.AttackMontageEndedHandle.IsValid())
	{
		InstanceData.Enemy->OnAttackMontageEnded.Remove(InstanceData.AttackMontageEndedHandle);
		InstanceData.AttackMontageEndedHandle.Reset();
	}

	if (InstanceData.ActionComponent
		&& ExecuteAttackIsStartedActionRunning(
			*InstanceData.ActionComponent,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
	{
		InstanceData.ActionComponent->CancelActiveAction(0.1f);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}

FMVSelectAndExecuteAttackTask::FMVSelectAndExecuteAttackTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVSelectAndExecuteAttackTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ActionComponent = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;
	InstanceData.SelectedAttack = FMVAICombatActionCandidate();

	APawn* Owner = ExecuteAttackResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!ExecuteAttackSelectCandidate(InstanceData.CombatContext, InstanceData.Candidates, InstanceData.SelectedAttack))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (!InstanceData.ActionComponent
		|| !ExecuteAttackTryStartAction(
			*InstanceData.ActionComponent,
			InstanceData.SelectedAttack,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!ExecuteAttackStartCooldown(*Owner, InstanceData.SelectedAttack))
	{
		InstanceData.ActionComponent->CancelActiveAction(0.0f);
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.LastAttackTag = MVAICombat::MakeActionTag(InstanceData.SelectedAttack);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVSelectAndExecuteAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	return ExecuteAttackIsStartedActionRunning(
		*InstanceData.ActionComponent,
		InstanceData.StartedActionTableName,
		InstanceData.StartedActionRowName)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVSelectAndExecuteAttackTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.ActionComponent
		&& ExecuteAttackIsStartedActionRunning(
			*InstanceData.ActionComponent,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
	{
		InstanceData.ActionComponent->CancelActiveAction(0.1f);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
