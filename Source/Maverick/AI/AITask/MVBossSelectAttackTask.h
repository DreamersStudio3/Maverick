#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeTypes.h"
#include "MVBossSelectAttackTask.generated.h"

/** 공격 후보 이름과 실행할 하위 State의 연결, DataTable 행은 하위 Task 소유 */
USTRUCT(BlueprintType)
struct FMVBossAttackChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Attack")
	FName AttackName;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FStateTreeStateLink AttackState;
};

/** BossAttack 진입별 선택 결과와 전이 요청 상태 */
USTRUCT()
struct FMVBossSelectAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TArray<FMVBossAttackChoice> Choices;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0", Units = "cm"))
	float NormalAttackRange = 300.0f;

	// Choices의 AttackName 지정, None이면 거리와 무관하게 배열 순서 선택
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FName OutOfRangeAttackName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FStateTreeStateLink ResonanceState;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	FName SelectedAttack;

	int32 SelectedIndex = INDEX_NONE;
	bool bTransitionRequested = false;
	bool bResonanceSelected = false;
};

/**
 * BossAttack 진입 시 범위 밖은 지정 후보 우선, 그 외에는 Choices 배열 순서로 선택 후 다음 Tick에서 전이
 * AMVAIController의 다음 후보 위치 보존, 순서 선택 시에만 전진; 범위 밖 예외는 순서 위치 유지
 * 공명 폭주 예약 시 ResonanceState 우선·순서 위치 유지; Pawn·TargetActor 원점 거리 사용
 * 부모는 TryEnterState 설정 필수, 유지 진입 중 재선택 없음; 공격 실행·사거리 검사는 하위 Task 책임
 */
USTRUCT(meta = (DisplayName = "Boss Select Attack"))
struct MAVERICK_API FMVBossSelectAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FMVBossSelectAttackTaskInstanceData;

	FMVBossSelectAttackTask();
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};
