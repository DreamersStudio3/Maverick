#pragma once

#include "CoreMinimal.h"
#include "MVCombatActionTypes.generated.h"

UENUM(BlueprintType)
enum class EMVCombatActionTypes : uint8
{
	None UMETA(DisplayName = "None"),
	LightAttack UMETA(DisplayName = "LightAttack"),
	HeavyAttack UMETA(DisplayName = "HeavyAttack"),
	ChargeSkill UMETA(DisplayName = "ChargeSkill"),
	Skill UMETA(DisplayName = "Skill"),
	Dodge UMETA(DisplayName = "Dodge"),
	Guard UMETA(DisplayName = "Guard"),
	UseComsumable UMETA(DisplayName = "UseComsumable")
};

UENUM(BlueprintType)
enum class EMVCombatAttackTypes : uint8
{
	LightAttack UMETA(DisplayName = "LightAttack"),
	HeavyAttack UMETA(DisplayName = "HeavyAttack"),
	Skill UMETA(DisplayName = "Skill"),
	StackedSkill UMETA(DisplayName = "StackedSkill")
};
