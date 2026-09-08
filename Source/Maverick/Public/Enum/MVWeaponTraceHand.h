#pragma once

#include "CoreMinimal.h"
#include "MVWeaponTraceHand.generated.h"

UENUM(BlueprintType)
enum class EMVWeaponTraceHand : uint8
{
	None UMETA(DisplayName = "None"),
	Right UMETA(DisplayName = "Right"),
	Left UMETA(DisplayName = "Left"),
	Both UMETA(DisplayName = "Both")
};