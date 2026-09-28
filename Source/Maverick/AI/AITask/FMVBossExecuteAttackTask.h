#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "Engine/DataTable.h"
#include "FMVBossExecuteAttackTask.generated.h"

class UMVActionComponent;

USTRUCT()
struct FMVBossExecuteAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	FDataTableRowHandle AttackRow;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	float AttackRange = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Context")
	FMVAICombatContext CombatContext;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

/**
 * 보스 공격 row 하나를 ActionComponent에서 실행하고 종료까지 감시하는 StateTree Task
 * 공격 선택·페이즈 판정은 상위 StateTree와 Condition 책임
 */
USTRUCT(meta = (DisplayName = "Boss Execute Attack"))
struct FMVBossExecuteAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVBossExecuteAttackTaskInstanceData;

	FMVBossExecuteAttackTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;
	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
