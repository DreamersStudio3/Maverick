#pragma once

#include "CoreMinimal.h"
#include "MVEquipmentEnums.generated.h"

UENUM(BlueprintType)
enum class EMVEquippedStyle : uint8
{
	BareHand UMETA(DisplayName = "Bare Hand"),
	OneHand UMETA(DisplayName = "One Hand"),
	TwoHand UMETA(DisplayName = "Two Hand")
};

UENUM(BlueprintType)
enum class EMVWeaponRangeType : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged")
};
