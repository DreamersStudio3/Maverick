#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "MVStrafeMoveTask.generated.h"

USTRUCT()
struct FMVStrafeMoveTaskInstanceData
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, Category = "Input")
	float MoveDuration = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	float TraceDistance = 300.0f;
	
	UPROPERTY()
	FVector ChosenDirection = FVector::ZeroVector;
	
	UPROPERTY()
	float ElapsedTime = 0.0f;
};

USTRUCT()
struct FMVStrafeMoveTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using InstanceDataType = FMVStrafeMoveTaskInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return InstanceDataType::StaticStruct(); 
	}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};
