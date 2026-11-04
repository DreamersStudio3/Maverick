#pragma once

#include "CoreMinimal.h"
#include "MVChainAdvancePolicy.generated.h"

UENUM(BlueprintType)
enum class EMVChainAdvancePolicy : uint8
{
	Immediate UMETA(DisplayName = "Immediate"),
	OnHitConfirmed UMETA(DisplayName = "On Hit Confirmed")
};
