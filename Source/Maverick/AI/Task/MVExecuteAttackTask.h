#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
#include "AI/Enum/MVAttackDirection.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVExecuteAttackTask.generated.h"

class AMVEnemy;
class UMVActionComponent;

USTRUCT()
struct FMVExecuteFixedAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	FMVAICombatActionCandidate Attack;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	EMVAttackDirection FallbackAttackDirection = EMVAttackDirection::Forward;

	UPROPERTY(EditAnywhere, Category = "Output")
	FName LastAttackTag = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AMVEnemy> Enemy = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
	int32 AttackInstanceId = INDEX_NONE;
	FDelegateHandle AttackMontageEndedHandle;
};

USTRUCT(meta = (DisplayName = "Execute Fixed Attack Task"))
struct FMVExecuteFixedAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVExecuteFixedAttackTaskInstanceData;

	FMVExecuteFixedAttackTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FMVSelectAndExecuteAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Context")
	FMVAICombatContext CombatContext;

	UPROPERTY(EditAnywhere, Category = "Input|Selection")
	EMVAICombatAttackSelectionMode SelectionMode = EMVAICombatAttackSelectionMode::SkillAttack;

	UPROPERTY(EditAnywhere, Category = "Input|Selection")
	TArray<FMVAICombatActionCandidate> Candidates;

	UPROPERTY(EditAnywhere, Category = "Output")
	FMVAICombatActionCandidate SelectedAttack;

	UPROPERTY(EditAnywhere, Category = "Output")
	FName LastAttackTag = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

USTRUCT(meta = (DisplayName = "Select And Execute Attack Task"))
struct FMVSelectAndExecuteAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVSelectAndExecuteAttackTaskInstanceData;

	FMVSelectAndExecuteAttackTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
