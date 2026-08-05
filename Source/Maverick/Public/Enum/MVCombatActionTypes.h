#pragma once

#include "CoreMinimal.h"
#include "MVCombatActionTypes.generated.h"

UENUM(BlueprintType)
enum class EMVCombatActionTypes : uint8
{
	None UMETA(DisplayName = "None"),
	LightAttack UMETA(DisplayName = "LightAttack"),
	HeavyAttack UMETA(DisplayName = "HeavyAttack"),
	ChargeAttack UMETA(DisplayName = "ChargeAttack"),
	Skill UMETA(DisplayName = "Skill"),
	Dodge UMETA(DisplayName = "Dodge"),
	Guard UMETA(DisplayName = "Guard"),
	UseComsumable UMETA(DisplayName = "UseComsumable"),
	SprintLightAttack UMETA(DisplayName = "SprintLightAttack"),
	SprintHeavyAttack UMETA(DisplayName = "SprintHeavyAttack"),
	DodgeLightAttack UMETA(DisplayName = "DodgeLightAttack"),
	DodgeHeavyAttack UMETA(DisplayName = "DodgeHeavyAttack")
};

UENUM(BlueprintType)
enum class EMVCombatAttackTypes : uint8
{
	LightAttack UMETA(DisplayName = "LightAttack"),
	HeavyAttack UMETA(DisplayName = "HeavyAttack"),
	ChargeAttack UMETA(DisplayName = "ChargeAttack"),
	Skill UMETA(DisplayName = "Skill"),
	SprintLightAttack UMETA(DisplayName = "SprintLightAttack"),
	SprintHeavyAttack UMETA(DisplayName = "SprintHeavyAttack"),
	DodgeLightAttack UMETA(DisplayName = "DodgeLightAttack"),
	DodgeHeavyAttack UMETA(DisplayName = "DodgeHeavyAttack")
};
