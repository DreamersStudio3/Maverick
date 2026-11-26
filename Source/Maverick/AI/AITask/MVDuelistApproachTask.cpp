#include "AI/AITask/MVDuelistApproachTask.h"
#include "AI/Controller/MVAIController.h"
#include "Combat/Boss/Duelist/MVDuelistApproachComponent.h"
#include "Combat/Boss/Duelist/MVDuelistDodgeChanceComponent.h"
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
	if (Approach->IsInAttackRange(Data.Target.Get()) && Controller->LineOfSightTo(Data.Target.Get()))
		return EStateTreeRunStatus::Succeeded;
	return Approach->IsMoving() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

void FMVDuelistApproachTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (auto* Approach = Context.GetInstanceData(*this).Approach.Get()) Approach->StopApproach();
}

EStateTreeRunStatus FMVDuelistWaitForEvasion::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const auto* Controller = Cast<AMVAIController>(Context.GetOwner());
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	Context.GetInstanceData(*this).Dodge = Pawn ? Pawn->FindComponentByClass<UMVDuelistDodgeChanceComponent>() : nullptr;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVDuelistWaitForEvasion::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	const auto* Dodge = Context.GetInstanceData(*this).Dodge.Get();
	return Dodge && Dodge->IsEvasionPlaying() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}