#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "MVAIPatrolTask.generated.h"

class AAIController;
class APawn;

USTRUCT()
struct MAVERICK_API FMVAIPatrolTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Patrol", meta = (Units = "cm"))
	float Radius = 900.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Patrol", meta = (Units = "cm"))
	float AcceptanceRadius = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bPatrolling = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bLastMoveSucceeded = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector Destination = FVector::ZeroVector;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> Controller = nullptr;

	bool bMoveRequested = false;
};

/**
 * 현재 StateTree 상태가 살아 있는 동안 패트롤 이동 요청 하나를 소유하는 Task
 * Enter에서 목적지·이동 요청 1회 생성, Tick에서 완료 대기; 대상 없는 동안 재요청 없음
 * AMVAIController의 유효한 TargetActor 발견 시 시작 생략 또는 다음 Tick에서 이동 중단·Succeeded 반환
 * 대상 발견 중단은 bLastMoveSucceeded false, 후속 Chase 선택은 StateTree 전이 책임
 * Exit에서 미완료 이동 정리, 다른 AIController 유형은 기존 순찰 경로 유지
 */
USTRUCT(meta = (DisplayName = "AI Patrol"))
struct MAVERICK_API FMVAIPatrolTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVAIPatrolTaskInstanceData;

	FMVAIPatrolTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		float DeltaTime) const override;
	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
