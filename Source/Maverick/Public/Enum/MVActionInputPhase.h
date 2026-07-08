#pragma once

#include "CoreMinimal.h"
#include "MVActionInputPhase.generated.h"

UENUM(BlueprintType)
enum class EMVActionInputPhase : uint8
{
	Started UMETA(DisplayName = "Started"),
	Triggered UMETA(DisplayName = "Triggered"),
	Completed UMETA(DisplayName = "Completed"),
	Canceled UMETA(DisplayName = "Canceled")
};
