#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "Engine/DataTable.h"
#include "FMVBossExecuteAttackTask.generated.h"

class UMVActionComponent;
class UAnimInstance;
class UAnimMontage;

/** 보스 선택용 몽타주 행; CombatAttackRow 지정 시 CombatComponent의 Ability 실행 경로 사용 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVTutorialBossSkillRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bStopOnExit = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDataTableRowHandle CombatAttackRow;
};

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

	UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> AnimInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveMontage = nullptr;

	bool bCustomMontageStopOnExit = true;
	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

/**
 * 보스 공격 행 실행·종료 감시 Task, 전투 행은 CombatComponent의 Ability 경로 사용
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
