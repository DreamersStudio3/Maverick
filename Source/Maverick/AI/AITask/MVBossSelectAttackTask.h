#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeTypes.h"
#include "MVBossSelectAttackTask.generated.h"

/** 공격 행 이름과 실행할 하위 State의 연결 */
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

	UPROPERTY(VisibleAnywhere, Category = "Output")
	FName SelectedAttack;

	int32 SelectedIndex = INDEX_NONE;
	bool bTransitionRequested = false;
};

/**
 * BossAttack 진입 시 후보 하나를 균등 선택하고 다음 Tick에서 하위 공격 State로 전이
 * 부모 상태 유지 중 재선택 없음; 공격 완료 여부는 하위 실행 Task 책임
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
