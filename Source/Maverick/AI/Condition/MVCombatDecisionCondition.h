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
