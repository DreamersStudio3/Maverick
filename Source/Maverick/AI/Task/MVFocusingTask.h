#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVFocusingTask.generated.h"

USTRUCT()
struct FMVFocusingTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Target")
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter|TurnSpeed")
	float TurnSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Input|bCanFocus")
	bool bCanFocus = true;
};

USTRUCT(meta = (DisplayName = "Focusing Task"))
struct FMVFocusingTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVFocusingTaskInstanceData;

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
};
