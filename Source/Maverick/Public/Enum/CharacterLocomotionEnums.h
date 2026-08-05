#pragma once

#include "CoreMinimal.h"
#include "CharacterLocomotionEnums.generated.h"

UENUM(BlueprintType)
enum class ELocomotionDirection : uint8
{
	F UMETA(DisplayName = "F"),
	FR UMETA(DisplayName = "FR"),
	R UMETA(DisplayName = "R"),
	BR UMETA(DisplayName = "BR"),
	B UMETA(DisplayName = "B"),
	BL UMETA(DisplayName = "BL"),
	L UMETA(DisplayName = "L"),
	FL UMETA(DisplayName = "FL")
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting")
};
