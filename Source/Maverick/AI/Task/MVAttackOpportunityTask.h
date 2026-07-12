#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
#include "StateTreeTaskBase.h"
#include "MVAttackOpportunityTask.generated.h"

USTRUCT()
struct FMVAttackOpportunityTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Context")
	FMVAICombatContext CombatContext;

	UPROPERTY(EditAnywhere, Category = "Input|Range", meta = (ClampMin = "0.0"))
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Range", meta = (ClampMin = "0.0"))
	float MaxDistance = 900.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Angle", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxAbsAngle = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName RequiredReadyActionId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	bool bRequireAttackCadenceReady = true;

	UPROPERTY(EditAnywhere, Category = "Input|Condition")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, Category = "Input|Condition")
	bool bFailWhenActionRunning = true;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bCanAttack = false;
};

/**
 * Completes when the current combat context can interrupt approach movement
 * and transition into an attack state.
 *
 * Intended to run beside MoveTo in a StateTree state whose task completion
 * mode is Any, so ranged or special attacks can start before the MoveTo target
 * location is reached.
 */
USTRUCT(meta = (DisplayName = "Attack Opportunity Task"))
struct FMVAttackOpportunityTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVAttackOpportunityTaskInstanceData;

	FMVAttackOpportunityTask();

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
};
