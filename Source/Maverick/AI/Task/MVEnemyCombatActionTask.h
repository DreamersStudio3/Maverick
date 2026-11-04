#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
#include "Enum/MVSkillSlot.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVEnemyCombatActionTask.generated.h"

class UMVActionComponent;
class UMVCombatComponent;

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

	UPROPERTY(EditAnywhere, Category = "Input|Combat")
	EMVSkillSlot SkillSlot = EMVSkillSlot::QSkill;

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
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMVCombatComponent> CombatComponent = nullptr;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
	bool bSectionJumpRequested = false;
};

/**
 * CombatComponent의 명시적 기본 공격 타입 또는 스킬 슬롯 API를 호출하는 Enemy StateTree Task.
 * 액션 Row와 Ability 구성은 WeaponComponent에서 갱신된 CombatComponent 런타임 맵을 사용하고,
 * StateTree는 공격 종류·스킬 슬롯 선택과 후속 전이만 소유한다.
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
