#include "AI/AITask/MVDuelistApproachTask.h"
#include "AI/Controller/MVAIController.h"
#include "Combat/Boss/Duelist/MVDuelistApproachComponent.h"
#include "StateTreeExecutionContext.h"

static UMVDuelistApproachComponent* DuelistApproachFromController(const AMVAIController* Controller)
{
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	return Pawn ? Pawn->FindComponentByClass<UMVDuelistApproachComponent>() : nullptr;
}

bool FMVDuelistCanApproach::TestCondition(FStateTreeExecutionContext& Context) const
{
	const auto* Controller = Cast<AMVAIController>(Context.GetOwner());
	const auto* Approach = DuelistApproachFromController(Controller);
	FVector End;
	return Approach && Approach->FindDestination(Controller->TargetActor, End)
		&& (!Context.GetInstanceData(*this).bRequireLineOfSight || Controller->LineOfSightTo(Controller->TargetActor));
}

FMVDuelistApproachTask::FMVDuelistApproachTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVDuelistApproachTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	auto* Controller = Cast<AMVAIController>(Context.GetOwner());
	auto& Data = Context.GetInstanceData(*this);
	Data.Approach = DuelistApproachFromController(Controller);
	Data.Target = Controller ? Controller->TargetActor : nullptr;
	return Data.Approach.IsValid() && Data.Approach->StartApproach(Data.Target.Get())
		? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FMVDuelistApproachTask::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	const auto& Data = Context.GetInstanceData(*this);
	const auto* Controller = Cast<AMVAIController>(Context.GetOwner());
	const auto* Approach = Data.Approach.Get();
	if (!Controller || !Approach || Controller->TargetActor != Data.Target.Get() || !Approach->CanContinue(Data.Target.Get()))
		return EStateTreeRunStatus::Failed;
	Approach->TurnTowards(Data.Target.Get(), DeltaTime);
	if (Approach->IsMoving()) return EStateTreeRunStatus::Running;
	return Approach->GetOwner()->GetDistanceTo(Data.Target.Get()) <= Approach->AttackRange && Controller->LineOfSightTo(Data.Target.Get())
		? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

void FMVDuelistApproachTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (auto* Approach = Context.GetInstanceData(*this).Approach.Get()) Approach->StopApproach();
}
