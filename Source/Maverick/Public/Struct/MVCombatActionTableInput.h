#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Public/Enum/MVEquipmentEnums.h"

#include "MVCombatActionTableInput.generated.h"

// This Struct is for the Chooser Table Input of Combat Component
// Requires Current Weapon Style and ActionType tag


USTRUCT(BlueprintType)
struct FMVCombatActionTableInput
{
	GENERATED_BODY()

	FMVCombatActionTableInput()
		: CurrentWeaponStyle(EMVEquippedStyle::BareHand), ActionType()
	{

	}

	UPROPERTY(BlueprintReadWrite)
	EMVEquippedStyle CurrentWeaponStyle;

	UPROPERTY(BlueprintReadWrite, meta = (Categories = "Action.Combat"))
	FGameplayTag ActionType;

};
