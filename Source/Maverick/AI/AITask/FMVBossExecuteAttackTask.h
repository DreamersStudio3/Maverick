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

	// 0 이하: 거리 제한 없이 실행, 양수: 해당 cm 범위 안에서만 실행
	UPROPERTY(EditAnywhere, Category = "Input|Attack", meta = (ClampMin = "0", Units = "cm"))
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
 * AttackRange 양수는 원점 거리 검사, 0 이하는 거리 제한 해제; 대상·행 검증 유지
 * 실행 종료·범위 생략 시 성공 전이 요청, Exit에서 시작한 액션만 정리; 공격 선택·페이즈는 상위 트리 책임
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
