#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVAIBossDefaultGlobalTask.generated.h"

/**
 * 보스별 실행 데이터와 대상 거리·사망 여부 출력
 * Owner는 보스, Target은 AttackTarget 바인딩 대상; DistanceToTarget은 bHasTarget이 참일 때만 유효한 3차원 거리(cm)
 * bIsDead는 대상 유무와 무관한 StatComponent 사망 판정의 읽기 전용 출력
 */
USTRUCT()
struct MAVERICK_API FMVAIBossDefaultGlobalTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (Units = "cm"))
	float DistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasTarget = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsDead = false;
};

/**
 * Enter와 매 Tick에 대상 거리 및 StatComponent의 사망 판정 조회, StateTree 실행 동안 Running 유지
 * 대상 탐색·사망 처리 소유권 없음; 대상 무효 시 거리 0 및 bHasTarget false, StatComponent 부재 시 bIsDead false
 */
USTRUCT(meta = (DisplayName = "AI Boss Default Global Task"))
struct MAVERICK_API FMVAIBossDefaultGlobalTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVAIBossDefaultGlobalTaskInstanceData;

	FMVAIBossDefaultGlobalTask();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

private:
	static void UpdateSnapshot(FInstanceDataType& InstanceData);
};
