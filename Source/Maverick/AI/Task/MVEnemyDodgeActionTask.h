#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StateTreeTaskBase.h"
#include "Struct/MVAIDodgeTypes.h"
#include "MVEnemyDodgeActionTask.generated.h"

class APawn;
class UMVActionComponent;

USTRUCT()
struct FMVEnemyDodgeActionTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Dodge")
	FMVAIDodgeRequest DodgeRequest;

	UPROPERTY(EditAnywhere, Category = "Input|Dodge")
	EMVActionInputDirection DefaultDirection = EMVActionInputDirection::Back;

	UPROPERTY(EditAnywhere, Category = "Input|Dodge")
	FDataTableRowHandle ForwardDodgeActionRow;

	UPROPERTY(EditAnywhere, Category = "Input|Dodge")
	FDataTableRowHandle BackDodgeActionRow;

	UPROPERTY(EditAnywhere, Category = "Input|Dodge")
	FDataTableRowHandle LeftDodgeActionRow;

	UPROPERTY(EditAnywhere, Category = "Input|Dodge")
	FDataTableRowHandle RightDodgeActionRow;

	UPROPERTY(EditAnywhere, Category = "Input|Dodge")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	bool bTransitionFromCurrentAction = true;

	UPROPERTY(EditAnywhere, Category = "Input|Action", meta = (ClampMin = "0.0", EditCondition = "bTransitionFromCurrentAction"))
	float TransitionBlendOutTime = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bWaitForActionEnd = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bCancelActionOnExit = false;

	UPROPERTY(EditAnywhere, Category = "Input|Task", meta = (ClampMin = "0.0", EditCondition = "bCancelActionOnExit"))
	float ExitCancelBlendOutTime = 0.1f;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	bool bStarted = false;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	EMVActionInputDirection ResolvedDirection = EMVActionInputDirection::Back;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

/**
 * Executes an enemy dodge action selected from a StateTree dodge payload.
 *
 * The task does not decide whether dodging is tactically appropriate. It only
 * receives a requested direction, resolves the matching dodge action row, and
 * asks ActionComponent to run that row.
 */
USTRUCT(meta = (DisplayName = "Enemy Dodge Action Task"))
struct FMVEnemyDodgeActionTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVEnemyDodgeActionTaskInstanceData;

	FMVEnemyDodgeActionTask();

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
