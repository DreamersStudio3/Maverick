#include "AI/Task/MVEnemyCombatActionTask.h"

#include "AI/MVActionCooldownComponent.h"
#include "AIController.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Components/MVActionComponent.h"
#include "StateTreeExecutionContext.h"

namespace
{
APawn* EnemyCombatActionTaskResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

bool EnemyCombatActionTaskIsStartedActionRunning(const FMVEnemyCombatActionTaskInstanceData& InstanceData)
{
	return InstanceData.ActionComponent
		&& InstanceData.ActionComponent->IsActionRunning()
		&& (InstanceData.StartedActionTableName.IsNone()
			|| InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName)
		&& (InstanceData.StartedActionRowName.IsNone()
			|| InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName);
}

FName EnemyCombatActionTaskMakeActionTag(const EMVEnemyCombatActionKind ActionKind, const int32 ActionIndex)
{
	switch (ActionKind)
	{
	case EMVEnemyCombatActionKind::HeavyAttack:
		return FName(*FString::Printf(TEXT("HeavyAttack%d"), ActionIndex));

	case EMVEnemyCombatActionKind::Skill:
		return FName(*FString::Printf(TEXT("Skill%d"), ActionIndex));

	default:
		return NAME_None;
	}
}

bool EnemyCombatActionTaskTryStartAction(FMVEnemyCombatActionTaskInstanceData& InstanceData)
{
	if (!InstanceData.Enemy)
	{
		return false;
	}

	switch (InstanceData.ActionKind)
	{
	case EMVEnemyCombatActionKind::HeavyAttack:
		return InstanceData.Enemy->TryHeavyAttack(
			InstanceData.SkillIndex,
			InstanceData.StartSection);

	case EMVEnemyCombatActionKind::Skill:
		return InstanceData.Enemy->TrySkillAttack(
			InstanceData.SkillIndex,
			InstanceData.StartSection);

	default:
		return false;
	}
}

bool EnemyCombatActionTaskTryStartCooldown(APawn& Owner, const FName CooldownActionId)
{
	if (CooldownActionId.IsNone())
	{
		return true;
	}

	UMVActionCooldownComponent* CooldownComponent = Owner.FindComponentByClass<UMVActionCooldownComponent>();
	return CooldownComponent && CooldownComponent->StartCooldown(CooldownActionId);
}

}

FMVEnemyCombatActionTask::FMVEnemyCombatActionTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVEnemyCombatActionTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.Enemy = nullptr;
	InstanceData.ActionComponent = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;
	InstanceData.LastAttackTag = NAME_None;

	APawn* Owner = EnemyCombatActionTaskResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Owner = Owner;
	InstanceData.Enemy = Cast<AMVEnemy>(Owner);
	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (!InstanceData.Enemy || !InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!EnemyCombatActionTaskTryStartAction(InstanceData))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.StartedActionTableName = InstanceData.ActionComponent->GetActiveActionTableName();
	InstanceData.StartedActionRowName = InstanceData.ActionComponent->GetActiveActionRowName();
	InstanceData.LastAttackTag = EnemyCombatActionTaskMakeActionTag(
		InstanceData.ActionKind,
		InstanceData.SkillIndex);

	if (!EnemyCombatActionTaskTryStartCooldown(*Owner, InstanceData.CooldownActionId))
	{
		InstanceData.ActionComponent->CancelActiveAction(0.0f);
		return EStateTreeRunStatus::Failed;
	}

	return InstanceData.bWaitForActionEnd ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FMVEnemyCombatActionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.bWaitForActionEnd)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EnemyCombatActionTaskIsStartedActionRunning(InstanceData)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVEnemyCombatActionTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.bWaitForActionEnd
		&& InstanceData.bCancelActionOnExit
		&& EnemyCombatActionTaskIsStartedActionRunning(InstanceData))
	{
		InstanceData.ActionComponent->CancelActiveAction(InstanceData.ExitCancelBlendOutTime);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
