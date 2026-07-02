#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "Struct/MVHitTypes.h"
#include "MVHitReactionTask.generated.h"

class UMVActionComponent;
class UMVHitReactionComponent;

USTRUCT()
struct FMVHitReactionTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Hit")
	FMVResolvedHitData HitData;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bWaitForActionEnd = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bFailWhenReactionNotStarted = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bRequireStateTreeHandledHitReactionType = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bCancelReactionOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bClearHitDataOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task", meta = (EditCondition = "bCancelReactionOnExit", ClampMin = "0.0"))
	float ExitCancelBlendOutTime = 0.1f;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMVHitReactionComponent> HitReactionComponent = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

/**
 * StateTree task that starts hit reaction through UMVHitReactionComponent.
 *
 * Bind `HitData` from the StateTree parameter or event data that carries
 * `FMVResolvedHitData`.
 */
USTRUCT(meta = (DisplayName = "Request Hit Reaction Task"))
struct FMVHitReactionTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVHitReactionTaskInstanceData;

	FMVHitReactionTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		float DeltaTime) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
