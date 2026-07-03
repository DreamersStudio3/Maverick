#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeConditionBase.h"
#include "MVGroggyEnterCondition.generated.h"

USTRUCT()
struct FMVGroggyEnterConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;
};

/**
 * StateTree enter condition for enemy groggy presentation.
 *
 * Resolves the owner pawn from a bound value or the owning AIController and
 * allows entry only while the owner's StatComponent reports active groggy
 * state.
 */
USTRUCT(meta = (DisplayName = "Groggy Enter Condition"))
struct FMVGroggyEnterCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVGroggyEnterConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
