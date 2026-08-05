#pragma once

#include "CoreMinimal.h"
#include "MVBossCombatArea.generated.h"

UENUM()
enum class EMVBossCombatArea : uint8
{
	OutsideArea UMETA(DisplayName = "Outside"),
	DefensiveArea UMETA(DisplayName = "Defensive"),
	OffensiveArea UMETA(DisplayName = "Offensive"),
};
