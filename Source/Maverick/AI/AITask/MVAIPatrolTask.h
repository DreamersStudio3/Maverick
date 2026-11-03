#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "MVAIPatrolTask.generated.h"

class AAIController;
class APawn;
class USplineComponent;
class UCharacterMovementComponent;

/** 순찰 입력·관측 값과 실행 중 적용한 이동 설정의 복구 상태, Task 인스턴스별 소유 */
USTRUCT()
struct MAVERICK_API FMVAIPatrolTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Patrol", meta = (ClampMin = "1", Units = "cm/s"))
	float PatrolSpeed = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Patrol", meta = (ClampMin = "1", Units = "cm"))
	float LookAheadDistance = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Patrol", meta = (ClampMin = "0", Units = "cm", ToolTip = "열린 Spline의 끝에서 방향을 전환할 거리"))
	float AcceptanceRadius = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Patrol", meta = (ClampMin = "1", Units = "deg/s"))
	float TurnRate = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bPatrolling = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector Destination = FVector::ZeroVector;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> Controller = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USplineComponent> Spline = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> Movement = nullptr;

	float Direction = 1.0f;
	float PreviousMaxWalkSpeed = 0.0f;
	FRotator PreviousRotationRate = FRotator::ZeroRotator;
	bool bPreviousUseControllerRotationYaw = false;
	bool bPreviousOrientRotationToMovement = false;
	bool bPreviousUseControllerDesiredRotation = false;
	bool bMovementSettingsApplied = false;
	FDelegateHandle MovementSettingsHandle;
};

/**
 * AMVEnemy.PatrolPath의 Spline을 CharacterMovement 입력으로 연속 추종하는 Task
 * Enter에서 경로 조회·기존 MoveTo 중단·locomotion 갱신 이후 순찰 설정 적용 구독
 * 닫힌 경로는 순환, 열린 경로는 끝에서 왕복; 지점 통과 중 Running 유지
 * TargetActor 발견 시 Succeeded 반환, Chase 전이는 StateTree 책임
 * 실패·대상 발견·Exit에서 기존 이동 설정 복구, 자동 장애물 우회는 경로 설계 범위
 */
USTRUCT(meta = (DisplayName = "AI Patrol"))
struct MAVERICK_API FMVAIPatrolTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVAIPatrolTaskInstanceData;
	FMVAIPatrolTask();
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
