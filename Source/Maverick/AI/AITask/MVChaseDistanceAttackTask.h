#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "MVChaseDistanceAttackTask.generated.h"

/** 추적 진입별 거리 유지 시간과 감시 대상, 거리 단위 cm·시간 단위 초 */
USTRUCT()
struct FMVChaseDistanceAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0", Units = "cm"))
	float MinAttackDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0", Units = "s"))
	float RequiredDuration = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	float ElapsedTime = 0.0f;

	TWeakObjectPtr<AActor> TrackedTarget;
};

/**
 * Chase에서 동일 대상과 최소 거리 이상을 연속 유지한 뒤 Succeeded 반환
 * AMVAIController 소유 StateTree 전용, 새 진입·대상 변경·거리 미달·대상 부재 시 시간 초기화
 * Chase의 Any 완료와 성공 전이가 Attack 목적지 소유, Exit에서 추적 이동 정리
 */
USTRUCT(meta = (DisplayName = "Chase Distance Attack"))
struct MAVERICK_API FMVChaseDistanceAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FMVChaseDistanceAttackTaskInstanceData;

	FMVChaseDistanceAttackTask();
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
