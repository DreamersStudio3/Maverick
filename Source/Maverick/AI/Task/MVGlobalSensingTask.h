#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "AI/Enum/MVBossCombatArea.h"
#include "AI/Enum/MVAttackDirection.h"
#include "MVGlobalSensingTask.generated.h"

USTRUCT()
struct FMVGlobalSensingTaskInstanceData
{
	GENERATED_BODY()
	
	//플레이어 
	UPROPERTY(EditAnywhere, Category = "Input|Target")
	TObjectPtr<AActor> Target;
	
	//보스
	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner;
	
	//각 Area 값 변수
	// UPROPERTY(EditAnywhere, Category = "Input|Area")
	// float OutsideArea;
	
	UPROPERTY(EditAnywhere, Category = "Input|Area")
	float DefensiveArea;
	
	UPROPERTY(EditAnywhere, Category = "Input|Area")
	float OffensiveArea;	

	UPROPERTY(EditAnywhere, Category = "Input|Debug")
	bool bDrawCombatAreaDebug = false;

	UPROPERTY(EditAnywhere, Category = "Input|Debug", meta = (EditCondition = "bDrawCombatAreaDebug", ClampMin = "0.0"))
	float CombatAreaDebugDuration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Debug", meta = (EditCondition = "bDrawCombatAreaDebug", ClampMin = "0.0"))
	float CombatAreaDebugHeightOffset = 5.0f;
	
	//플레이어와 거리
	UPROPERTY(EditAnywhere, Category = "Output")
	float DistanceToTarget = 0.0f;
	
	//플레이어가 위치한 방향
	UPROPERTY(EditAnywhere, Category = "Output")
	float AngleToTarget = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	EMVAttackDirection AttackDirection = EMVAttackDirection::Forward;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	EMVBossCombatArea CurrentArea;
	
	
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
		const FStateTreeTransitionResult& Transition
		) const override;
	
	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime
		) const override;
	
	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
		) const override;
	

public: 
		
};
