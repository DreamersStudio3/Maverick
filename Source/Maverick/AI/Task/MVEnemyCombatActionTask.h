#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVEnemyCombatActionTask.generated.h"

class AMVEnemy;
class UMVActionComponent;

UENUM(BlueprintType)
enum class EMVEnemyCombatActionKind : uint8
{
	HeavyAttack,
	Skill
};

USTRUCT()
struct FMVEnemyCombatActionTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Combat")
	EMVEnemyCombatActionKind ActionKind = EMVEnemyCombatActionKind::HeavyAttack;

	UPROPERTY(EditAnywhere, Category = "Input|Combat", meta = (ClampMin = "0"))
	int32 SkillIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName CooldownActionId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bWaitForActionEnd = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bCancelActionOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task", meta = (ClampMin = "0.0"))
	float ExitCancelBlendOutTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Output")
	FName LastAttackTag = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<AMVEnemy> Enemy = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

/**
 * Runs an enemy combat action through AMVEnemy's Blueprint-overridable bridge.
 *
 * The task only chooses between HeavyAttack and SkillIndex. Actual DataTable
 * selection stays in CombatComponent/Blueprint chooser logic, so StateTree does
 * not need to know concrete row names such as Counter, Sprint, or Airborne.
 */
USTRUCT(meta = (DisplayName = "Enemy Combat Action Task"))
struct FMVEnemyCombatActionTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVEnemyCombatActionTaskInstanceData;

	FMVEnemyCombatActionTask();

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
