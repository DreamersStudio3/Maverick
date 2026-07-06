#pragma once

#include "CoreMinimal.h"
#include "MVItemEnums.generated.h"

UENUM(BlueprintType)
enum class EMVItemType : uint8
{
	None UMETA(DisplayName = "None"),
	HealingPotion UMETA(DisplayName = "Healing Potion"),
	Equipment UMETA(DisplayName = "Equipment"),
	Quest UMETA(DisplayName = "Quest"),
	CurrencyCoupon UMETA(DisplayName = "Currency Coupon"),
	Material UMETA(DisplayName = "Material")
};

UENUM(BlueprintType)
enum class EMVEquipmentSlot : uint8
{
	None UMETA(DisplayName = "None"),
	Weapon UMETA(DisplayName = "Weapon"),
	Armor UMETA(DisplayName = "Armor"),
	Accessory UMETA(DisplayName = "Accessory")
};
