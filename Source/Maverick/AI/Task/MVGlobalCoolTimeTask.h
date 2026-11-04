#pragma once

#include "CoreMinimal.h"
#include "AI/MVActionCooldownComponent.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVGlobalCoolTimeTask.generated.h"

USTRUCT()
struct FMVGlobalCoolTimeTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	TArray<FMVActionCooldownDefinition> ActionCooldowns;

	UPROPERTY(EditAnywhere, Category = "Input|Query")
	FName QueryActionId = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsQueryActionReady = false;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	float QueryActionRemainingTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	TArray<FName> ReadyActionIds;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bAttackCadenceReady = true;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionCooldownComponent> CooldownComponent = nullptr;
};

/**
 * `UMVActionCooldownComponent`를 구성하고 시간 경과와 ready 목록을 갱신하는 독립 StateTree Task.
 * GlobalSensing Task도 같은 cooldown tick을 소유할 수 있으므로 한 StateTree에서 둘을 동시에 배치하지 않는다.
 */
USTRUCT(meta = (DisplayName = "Global Action Cooldown Task"))
struct FMVGlobalCoolTimeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FMVGlobalCoolTimeTaskInstanceData;

	FMVGlobalCoolTimeTask();
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
