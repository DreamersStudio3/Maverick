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

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown", meta = (ClampMin = "0.0"))
	float AttackCooldownSeconds = 1.5f;

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

/**
 * 타깃과 전투 환경을 감지해 `FMVAICombatContext`를 계속 갱신하는 StateTree 부모 Task.
 *
 * 거리·각도·LOS·이동 경로와 ActionComponent 상태를 모으고 `UMVActionCooldownComponent`를 구성·tick한다.
 * 같은 StateTree에서 Global Action Cooldown Task와 함께 tick 소유자가 되면 쿨다운이 이중 감소하므로
 * 둘 중 하나만 배치한다. 실제 Target, CombatContext와 LastAttackTag binding은 StateTree 에셋에서 확인한다.
 */
USTRUCT()
struct FMVGlobalSensingTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FMVGlobalSensingTaskInstanceData;

	FMVGlobalSensingTask();
	
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
