#include "AI/AITask/MVChaseDistanceAttackTask.h"

#include "AI/Controller/MVAIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

FMVChaseDistanceAttackTask::FMVChaseDistanceAttackTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVChaseDistanceAttackTask::EnterState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Sustained)
	{
		FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
		Data.ElapsedTime = 0.0f;
		Data.TrackedTarget.Reset();
	}
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVChaseDistanceAttackTask::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const AMVAIController* Controller = Cast<AMVAIController>(Context.GetOwner());
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	AActor* Target = Controller ? Controller->TargetActor : nullptr;
	if (Data.TrackedTarget.Get() != Target)
	{
		Data.ElapsedTime = 0.0f;
		Data.TrackedTarget = Target;
	}
	if (!IsValid(Pawn) || !IsValid(Target) || Target->IsActorBeingDestroyed()
		|| Pawn->GetDistanceTo(Target) < Data.MinAttackDistance)
	{
		Data.ElapsedTime = 0.0f;
		return EStateTreeRunStatus::Running;
	}
	Data.ElapsedTime += DeltaTime;
	return Data.ElapsedTime >= Data.RequiredDuration ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

void FMVChaseDistanceAttackTask::ExitState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Sustained)
	{
		if (AMVAIController* Controller = Cast<AMVAIController>(Context.GetOwner()))
		{
			Controller->StopMovement();
		}
	}
}
