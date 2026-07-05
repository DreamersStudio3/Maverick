#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVHealingPotionTypes.generated.h"

/**
 * 플레이어 회복약 슬롯의 런타임 상태.
 *
 * PlayerConsumableComponent가 이 상태를 소유하고 QuickSlotWidget에는 표시용 데이터로 변환해 전달한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHealingPotionRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HealingPotion", meta = (Categories = "Item.HealingPotion"))
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HealingPotion", meta = (ClampMin = "0.0"))
	float HealAmount = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HealingPotion", meta = (ClampMin = "0"))
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HealingPotion", meta = (ClampMin = "0"))
	int32 MaxCarryCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HealingPotion")
	bool bCanUse = false;
};
