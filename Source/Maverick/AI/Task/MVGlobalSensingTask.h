#pragma once

#include "CoreMinimal.h"
#include "AI/MVActionCooldownComponent.h"
#include "AI/MVAICombatTypes.h"
#include "AI/Enum/MVBossCombatArea.h"
#include "AI/Enum/MVAttackDirection.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVGlobalSensingTask.generated.h"

USTRUCT()
struct FMVGlobalSensingTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Input|Target")
	TObjectPtr<AActor> Target = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Input|Area")
	float DefensiveArea = 1200.0f;
	
	UPROPERTY(EditAnywhere, Category = "Input|Area")
	float OffensiveArea = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	float SprintTriggerDistance = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	float SprintMaxReach = 2200.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	float SprintAttackAngle = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	float AirborneChargeMaxReach = 1600.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	float AirborneChargeAngle = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	float BasicAttackMaxReach = 350.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Decision")
	float AttackAngleTolerance = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Path")
	float ForwardPathTraceDistance = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Path")
	float BackwardPathTraceDistance = 350.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Path")
	float StrafePathTraceDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	TArray<FMVActionCooldownDefinition> ActionCooldowns;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName AttackCadenceActionId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Pattern")
	int32 CurrentPhase = 0;

	UPROPERTY(EditAnywhere, Category = "Input|Pattern")
	bool bCounterWindow = false;

	UPROPERTY(EditAnywhere, Category = "Input|Pattern")
	bool bTargetCanBeAirborne = false;

	UPROPERTY(EditAnywhere, Category = "Input|Pattern")
	bool bTargetIsAirborne = false;

	UPROPERTY(EditAnywhere, Category = "Input|Pattern")
	bool bShouldUseAirborneCharge = false;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	FName LastAttackTag = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Life")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, Category = "Input|Debug")
	bool bDrawCombatAreaDebug = false;

	UPROPERTY(EditAnywhere, Category = "Input|Debug", meta = (EditCondition = "bDrawCombatAreaDebug", ClampMin = "0.0"))
	float CombatAreaDebugDuration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Debug", meta = (EditCondition = "bDrawCombatAreaDebug", ClampMin = "0.0"))
	float CombatAreaDebugHeightOffset = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	float DistanceToTarget = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	float AngleToTarget = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	EMVAttackDirection AttackDirection = EMVAttackDirection::Forward;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	EMVBossCombatArea CurrentArea = EMVBossCombatArea::OutsideArea;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasTarget = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasLineOfSight = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bActionRunning = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bSprintPathClear = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bAirborneChargePathClear = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bNeedAttackAngle = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bNeedClearAttackPath = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bStrafePathClear = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	TArray<FName> ReadyActionIds;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bAttackCadenceReady = true;

	UPROPERTY(EditAnywhere, Category = "Output")
	FMVAICombatContext CombatContext;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionCooldownComponent> CooldownComponent = nullptr;
};

USTRUCT()
struct FMVGlobalSensingTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FMVGlobalSensingTaskInstanceData;
	
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
