#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "MVChaseTargetTask.generated.h"

USTRUCT()
struct FMVChaseTargetTaskData 
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<APawn> Target;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<APawn> Owner;
		
};

USTRUCT()
struct FMVChaseTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using InstanceDataType = FMVChaseTargetTaskData;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return InstanceDataType::StaticStruct();
	};
	
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
		
};
