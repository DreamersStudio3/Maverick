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
 * Enter에서 내비게이션 목표를 한 번 생성하고, Running Tick에서 이동이 끝날 때까지 재요청하지 않음
 * State가 종료되면 Exit에서 해당 Controller의 이동을 정리해 다음 상태의 이동 요청과 충돌하지 않게 처리
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
