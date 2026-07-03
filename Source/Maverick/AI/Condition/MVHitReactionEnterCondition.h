#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeConditionBase.h"
#include "Struct/MVHitTypes.h"
#include "MVHitReactionEnterCondition.generated.h"

UENUM(BlueprintType)
enum class EMVHitReactionEnterTypeFilter : uint8
{
	NonGroggy,
	GroggyOnly,
	Any
};

USTRUCT()
struct FMVHitReactionEnterConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Hit")
	FMVResolvedHitData HitData;

	UPROPERTY(EditAnywhere, Category = "Input|Hit")
	bool bRequireOwnerAsVictim = true;

	UPROPERTY(EditAnywhere, Category = "Input|Hit")
	bool bRequireHitReactionType = true;

	UPROPERTY(EditAnywhere, Category = "Input|Hit")
	bool bRequireStateTreeHandledHitReactionType = true;

	UPROPERTY(EditAnywhere, Category = "Input|Hit")
	EMVHitReactionEnterTypeFilter HitReactionTypeFilter = EMVHitReactionEnterTypeFilter::NonGroggy;

	UPROPERTY(EditAnywhere, Category = "Input|Component")
	bool bRequireHitReactionComponent = true;
};

/**
 * StateTree enter condition for enemy hit reactions.
 *
 * Validates the hit reaction data before the tree enters a Hit state. Bind
 * `HitData` from the StateTree parameter or event data that carries
 * `FMVResolvedHitData`.
 */
USTRUCT(meta = (DisplayName = "Hit Reaction Enter Condition"))
struct FMVHitReactionEnterCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVHitReactionEnterConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
