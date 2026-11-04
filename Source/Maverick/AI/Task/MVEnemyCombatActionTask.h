#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
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

	UPROPERTY(EditAnywhere, Category = "Input|Context")
	FMVAICombatContext CombatContext;

	UPROPERTY(EditAnywhere, Category = "Input|Combat")
	EMVEnemyCombatActionKind ActionKind = EMVEnemyCombatActionKind::HeavyAttack;

	UPROPERTY(EditAnywhere, Category = "Input|Combat", meta = (ClampMin = "0", DisplayName = "Action Index"))
	int32 SkillIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Input|Combat")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Cooldown")
	FName CooldownActionId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bWaitForActionEnd = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bCancelActionOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task", meta = (ClampMin = "0.0"))
	float ExitCancelBlendOutTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Input|Section Jump")
	bool bJumpToSectionWhenInRange = false;

	UPROPERTY(EditAnywhere, Category = "Input|Section Jump", meta = (EditCondition = "bJumpToSectionWhenInRange"))
	FName JumpSectionName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Section Jump", meta = (ClampMin = "0.0", EditCondition = "bJumpToSectionWhenInRange"))
	float JumpDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Section Jump", meta = (EditCondition = "bJumpToSectionWhenInRange"))
	bool bStopMovementOnSectionJump = true;

	UPROPERTY(EditAnywhere, Category = "Output")
	FName LastAttackTag = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<AMVEnemy> Enemy = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
	bool bSectionJumpRequested = false;
};

/**
 * Runs a HeavyAttack or Skill row by index through CombatComponent, keeping
 * ability and trace setup in the combat domain while letting StateTree own
 * attack-state selection and follow-up transitions.
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
