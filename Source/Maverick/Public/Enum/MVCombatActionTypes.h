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

// 공격 유형
// 현재는 NormalAttack와 SkillAttack 두 가지로만 구분하지만
// 추후 구조를 여러가지로 확장 할 수 있음
UENUM(BlueprintType)
enum class EMVAttackTypes : uint8
{
	NormalAttack UMETA(DisplayName = "NormalAttack"),
	SkillAttack UMETA(DisplayName = "SkillAttack")
};
