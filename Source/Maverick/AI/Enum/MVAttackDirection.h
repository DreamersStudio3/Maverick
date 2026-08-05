#pragma once

#include "CoreMinimal.h"
#include "MVAttackDirection.generated.h"

UENUM(BlueprintType)
enum class EMVAttackDirection : uint8
{
	Forward UMETA(DisplayName = "Forward"),
	Backward UMETA(DisplayName = "Backward"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
};
