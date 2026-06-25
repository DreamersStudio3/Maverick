#pragma once

#include "CoreMinimal.h"
#include "Public/Enum/MVEquipmentEnums.h"
#include "Public/Enum/MVCombatActionTypes.h"

#include "MVCombatActionTableInput.generated.h"

// This Struct is for the Chooser Table Input of Combat Component
// Requires Current Weapon Style and ActionTypes


USTRUCT(BlueprintType)
struct FMVCombatActionTableInput
{
	GENERATED_BODY()

	FMVCombatActionTableInput()
		: CurrentWeaponStyle(EMVEquippedStyle::BareHand), ActionType(EMVCombatActionTypes::None)
	{

	}

	UPROPERTY(BlueprintReadWrite)
	EMVEquippedStyle CurrentWeaponStyle;

	UPROPERTY(BlueprintReadWrite)
	EMVCombatActionTypes ActionType;

};