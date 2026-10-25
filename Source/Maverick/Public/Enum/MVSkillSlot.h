#pragma once

#include "CoreMinimal.h"
#include "MVSkillSlot.generated.h"

UENUM(BlueprintType)
enum class EMVSkillSlot : uint8
{
	NONE UMETA(DisplayName = "NONE"),
	PassiveSkill UMETA(DisplayName = "PassiveSkill"),
	QSkill UMETA(DisplayName = "QSkill"),
	WSkill UMETA(DisplayName = "WSkill"),
	ESkill UMETA(DisplayName = "ESkill"),
	RSkill UMETA(DisplayName = "RSkill")
};
