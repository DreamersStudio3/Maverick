#pragma once

#include "CoreMinimal.h"
#include "AI/Enum/MVAttackDirection.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVAttackTask.generated.h"

class UMVActionComponent;
class UMVCombatComponent;

USTRUCT()
struct FMVAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APawn> Pawn = nullptr;

	UPROPERTY()
	TObjectPtr<UMVCombatComponent> CombatComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	EMVAttackDirection AttackDirection = EMVAttackDirection::Forward;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

USTRUCT(meta = (DisplayName = "Attack Task"))
struct FMVAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVAttackTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
