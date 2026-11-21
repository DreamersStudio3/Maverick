#pragma once

#include "CoreMinimal.h"
#include "MVCombatActionTypes.generated.h"

UENUM(BlueprintType)
enum class EMVCombatActionTypes : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	LightAttack = 1 UMETA(DisplayName = "LightAttack"),
	ChargeAttack = 3 UMETA(DisplayName = "ChargeAttack"),
	Skill = 4 UMETA(DisplayName = "Skill"),
	Dodge = 5 UMETA(DisplayName = "Dodge"),
	Guard = 6 UMETA(DisplayName = "Guard"),
	UseComsumable = 7 UMETA(DisplayName = "UseComsumable"),
	SprintLightAttack = 8 UMETA(DisplayName = "SprintLightAttack"),
	DodgeLightAttack = 10 UMETA(DisplayName = "DodgeLightAttack")
};

UENUM(BlueprintType)
enum class EMVCombatAttackTypes : uint8
{
	LightAttack = 0 UMETA(DisplayName = "LightAttack"),
	ChargeAttack = 2 UMETA(DisplayName = "ChargeAttack"),
	Skill = 3 UMETA(DisplayName = "Skill"),
	SprintLightAttack = 4 UMETA(DisplayName = "SprintLightAttack"),
	DodgeLightAttack = 6 UMETA(DisplayName = "DodgeLightAttack")
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
