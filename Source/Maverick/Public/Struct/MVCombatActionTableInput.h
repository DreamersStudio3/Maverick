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

	void SetActionType(const FGameplayTag NewActionType)
	{
		ActionType = NewActionType;
		ActionTypeTags.Reset();
		if (ActionType.IsValid())
		{
			ActionTypeTags.AddTag(ActionType);
		}
	}

	void RefreshActionTypeTags()
	{
		if (!ActionType.IsValid())
		{
			return;
		}

		ActionTypeTags.Reset();
		ActionTypeTags.AddTag(ActionType);
	}

	UPROPERTY(BlueprintReadWrite)
	EMVEquippedStyle CurrentWeaponStyle;

	UPROPERTY(BlueprintReadWrite, meta = (Categories = "Action.Combat"))
	FGameplayTag ActionType;

	UPROPERTY(BlueprintReadWrite, meta = (Categories = "Action.Combat"))
	FGameplayTagContainer ActionTypeTags;

};
