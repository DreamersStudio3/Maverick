#include "AI/AITask/MVDuelistResonanceTask.h"

#include "AIController.h"
#include "Combat/Duelist/MVDuelistResonanceComponent.h"
#include "StateTreeExecutionContext.h"

FMVDuelistResonanceTask::FMVDuelistResonanceTask() { bShouldCallTick = true; }

EStateTreeRunStatus FMVDuelistResonanceTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (Transition.ChangeType == EStateTreeStateChangeType::Sustained) return EStateTreeRunStatus::Running;
	const AAIController* AI = Cast<AAIController>(Context.GetOwner());
	AActor* Actor = AI ? AI->GetPawn() : Cast<AActor>(Context.GetOwner());
	Data.Resonance = Actor ? Actor->FindComponentByClass<UMVDuelistResonanceComponent>() : nullptr;
	return Data.Resonance && Data.Resonance->StartResonance() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FMVDuelistResonanceTask::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (Data.Resonance && Data.Resonance->IsResonanceActive()) return EStateTreeRunStatus::Running;
	Context.RequestTransition(FStateTreeTransitionRequest(FStateTreeStateHandle::Succeeded));
	return EStateTreeRunStatus::Succeeded;
}

void FMVDuelistResonanceTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType == EStateTreeStateChangeType::Sustained) return;
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (Data.Resonance) Data.Resonance->CancelResonance();
}
