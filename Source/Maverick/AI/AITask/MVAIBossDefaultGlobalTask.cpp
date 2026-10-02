#include "MVAIBossDefaultGlobalTask.h"

#include "Components/MVStatComponent.h"
#include "StateTreeExecutionContext.h"

// 거리·대상 유효성·사망 상태를 매 Tick 조회하는 전역 Task, 이동·공격 실행은 다른 Task 책임
// Tick 호출과 입력 바인딩 갱신 활성화
FMVAIBossDefaultGlobalTask::FMVAIBossDefaultGlobalTask()
{
	bShouldCallTick = true;
	bShouldCopyBoundPropertiesOnTick = true;
}

// 최초 진입 시 상태 출력 초기화, 전역 조회를 계속 유지하도록 Running 반환
EStateTreeRunStatus FMVAIBossDefaultGlobalTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	UpdateSnapshot(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

// 갱신된 Owner·Target 바인딩 기준 상태 재조회, 자체 완료 전이 없이 Running 유지
EStateTreeRunStatus FMVAIBossDefaultGlobalTask::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	UpdateSnapshot(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

// 사망 여부는 StatComponent에서 조회, Owner·Target 유효 시 거리 계산, 무효 시 대상 없음·거리 0 출력
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
