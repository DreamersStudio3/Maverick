#include "AI/AITask/MVBossSelectAttackTask.h"

#include "StateTreeExecutionContext.h"

FMVBossSelectAttackTask::FMVBossSelectAttackTask()
{
	bShouldCallTick = true;
#if WITH_EDITORONLY_DATA
	bConsideredForCompletion = false;
#endif
}

EStateTreeRunStatus FMVBossSelectAttackTask::EnterState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (Transition.ChangeType == EStateTreeStateChangeType::Sustained)
	{
		return EStateTreeRunStatus::Running;
	}
	Data.SelectedAttack = NAME_None;
	Data.SelectedIndex = INDEX_NONE;
	Data.bTransitionRequested = false;
	if (Data.Choices.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossAttack] Select failed: no choices Owner=%s"), *GetNameSafe(Context.GetOwner()));
		return EStateTreeRunStatus::Failed;
	}
	Data.SelectedIndex = FMath::RandRange(0, Data.Choices.Num() - 1);
	Data.SelectedAttack = Data.Choices[Data.SelectedIndex].AttackName;
	UE_LOG(LogTemp, Display, TEXT("[BossAttack] Selected: Owner=%s Attack=%s"),
		*GetNameSafe(Context.GetOwner()), *Data.SelectedAttack.ToString());
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVBossSelectAttackTask::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!Data.bTransitionRequested)
	{
		Data.bTransitionRequested = true;
		if (!Data.Choices.IsValidIndex(Data.SelectedIndex))
		{
			Context.RequestTransition(FStateTreeTransitionRequest(FStateTreeStateHandle::Failed));
			return EStateTreeRunStatus::Failed;
		}
		Context.RequestTransition(FStateTreeTransitionRequest(Data.Choices[Data.SelectedIndex].AttackState));
	}
	return EStateTreeRunStatus::Running;
}
