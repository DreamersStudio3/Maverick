#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVStartActionCooldownTask.generated.h"

USTRUCT()
struct FMVStartActionCooldownTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName StartedActionTableName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName StartedActionRowName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	bool bRequireStartedAction = true;
};

USTRUCT(meta = (DisplayName = "Start Action Cooldown Task"))
struct FMVStartActionCooldownTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVStartActionCooldownTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
