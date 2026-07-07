#pragma once

#include "CoreMinimal.h"
#include "AI/Enum/MVBossCombatArea.h"
#include "AI/MVAICombatTypes.h"
#include "Enum/MVCombatActionTypes.h"
#include "StateTreeConditionBase.h"
#include "MVCombatActionEnterCondition.generated.h"

UENUM()
enum class EMVCombatContextEnterMode : uint8
{
	Dead UMETA(DisplayName = "Dead"),
	Action UMETA(DisplayName = "Action"),
	MoveToTarget UMETA(DisplayName = "Move To Target"),
	Strafe UMETA(DisplayName = "Strafe"),
	Idle UMETA(DisplayName = "Idle")
};

USTRUCT()
struct FMVCombatActionEnterConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Context")
	FMVAICombatContext CombatContext;

	UPROPERTY(EditAnywhere, Category = "Input|Mode")
	EMVCombatContextEnterMode Mode = EMVCombatContextEnterMode::Action;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	EMVCombatActionTypes ActionType = EMVCombatActionTypes::HeavyAttack;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	FName CooldownActionId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Range")
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Range")
	float MaxDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Range", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxAbsAngle = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Area")
	bool bRequireCombatArea = false;

	UPROPERTY(EditAnywhere, Category = "Input|Area", meta = (EditCondition = "bRequireCombatArea"))
	EMVBossCombatArea RequiredCombatArea = EMVBossCombatArea::OffensiveArea;
};

/**
 * Checks whether a combat StateTree state may enter using the current AI combat
 * context. Attack cooldowns are read from CombatContext.ReadyActionIds.
 */
USTRUCT(meta = (DisplayName = "Combat Context Enter Condition"))
struct FMVCombatActionEnterCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVCombatActionEnterConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
