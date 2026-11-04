#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
#include "StateTreeConditionBase.h"
#include "MVCombatDecisionCondition.generated.h"

USTRUCT()
struct FMVCombatDecisionConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Context")
	FMVAICombatContext CombatContext;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	EMVAICombatDecisionState DesiredState = EMVAICombatDecisionState::Idle;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	FMVAICombatActionCondition CounterAttack;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	FMVAICombatActionCondition SprintAttack;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	FMVAICombatActionCondition AirborneChargeAttack;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	TArray<FMVAICombatActionCondition> SkillAttacks;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	TArray<FMVAICombatActionCondition> BasicAttacks;

	UPROPERTY(EditAnywhere, Category = "Input|Range")
	float SprintMaxReach = 2200.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Range")
	float CombatMaxDistance = 900.0f;
};

/**
 * 공유 전투 snapshot과 공격 후보를 비교해 목표 State 진입 가능 여부만 판정하는 조건.
 *
 * Dead 이외의 상태는 사망, 타깃 부재, 실행 중 Action에서 실패한다. 공격 가능 범위는 후보별 Min/MaxDistance와
 * MaxAbsAngle이 결정하며, `CombatMaxDistance`는 MoveToTarget과 Strafe를 나누는 이동 기준일 뿐 공격 사거리
 * 제한이 아니다. 여러 State가 동시에 성립할 때의 선택 순서는 StateTree 에셋이 소유한다.
 */
USTRUCT(meta = (DisplayName = "Combat Decision Condition"))
struct FMVCombatDecisionCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVCombatDecisionConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
