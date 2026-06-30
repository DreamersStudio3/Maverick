#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeConditionBase.h"
#include "MVActionCooldownReadyCondition.generated.h"

USTRUCT()
struct FMVActionCooldownReadyConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName CooldownActionId = NAME_None;
};

USTRUCT(meta = (DisplayName = "Action Cooldown Ready"))
struct FMVActionCooldownReadyCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVActionCooldownReadyConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
