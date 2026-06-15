#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Character/MVCharacterBase.h"
#include "MVBossLookAtTargetTask.generated.h"

USTRUCT()
struct FMVBossLookAtTargetTaskData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Input|Target")
	TObjectPtr<AActor> Target;
	
	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<AActor> Owner;
	
	UPROPERTY(EditAnywhere, Category = "Input|TurnSpeed")
	float TurnSpeed = 5.0f;;
		
};

USTRUCT()
struct FMVBossLookAtTargetTask : public FStateTreeTaskCommonBase
{
	
	GENERATED_BODY()
	
	using FInstanceData = FMVBossLookAtTargetTaskData;
	
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
