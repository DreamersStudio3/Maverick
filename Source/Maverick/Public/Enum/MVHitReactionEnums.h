#pragma once

#include "CoreMinimal.h"
#include "MVHitReactionEnums.generated.h"

UENUM(BlueprintType)
enum class EMVLyingFaceDirection : uint8
{
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

