#include "AI/AITask/MVBossSelectAttackTask.h"

#include "StateTreeExecutionContext.h"

// 진입당 공격 하나 선택·해당 상태로 전이 요청 담당, 공격 실행·완료 판정은 실행 Task 책임
// Tick 활성화, 에디터 설정에서는 이 Task를 상태 완료 판정 대상에서 제외
FMVBossSelectAttackTask::FMVBossSelectAttackTask()
{
	bShouldCallTick = true;
#if WITH_EDITORONLY_DATA
	bConsideredForCompletion = false;
#endif
}

// 새 진입 시 Choices에서 균등 무작위로 공격 하나 선택, 유지 진입에서는 재선택 생략
// 후보가 없으면 Failed, 선택 결과 저장 후 Running 반환; 실제 전이 요청은 Tick에서 처리
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

// 선택된 AttackState로 전이 1회 요청 후 Running 유지, 잘못된 선택 인덱스는 실패 전이 요청
// 첫 Tick까지 선택 전이 지연: 진입 시 기본 자식 상태 실행 차단은 별도 트리 구성 책임
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
