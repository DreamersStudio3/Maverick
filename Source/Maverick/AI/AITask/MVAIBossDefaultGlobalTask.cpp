#include "MVAIBossDefaultGlobalTask.h"

#include "Components/MVStatComponent.h"
#include "StateTreeExecutionContext.h"

FMVAIBossDefaultGlobalTask::FMVAIBossDefaultGlobalTask()
{
	bShouldCallTick = true;
	bShouldCopyBoundPropertiesOnTick = true;
}

EStateTreeRunStatus FMVAIBossDefaultGlobalTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	UpdateSnapshot(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVAIBossDefaultGlobalTask::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	UpdateSnapshot(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FMVAIBossDefaultGlobalTask::UpdateSnapshot(FInstanceDataType& InstanceData)
{
	// 사망 판정의 소유권은 StatComponent, 이 Task의 책임은 조회 결과 전달
	const UMVStatComponent* StatComponent = IsValid(InstanceData.Owner)
		? InstanceData.Owner->FindComponentByClass<UMVStatComponent>()
		: nullptr;
	InstanceData.bIsDead = StatComponent && StatComponent->IsDead();

	InstanceData.bHasTarget = IsValid(InstanceData.Owner) && IsValid(InstanceData.Target);
	InstanceData.DistanceToTarget = InstanceData.bHasTarget
		? InstanceData.Owner->GetDistanceTo(InstanceData.Target)
		: 0.0f;
}
