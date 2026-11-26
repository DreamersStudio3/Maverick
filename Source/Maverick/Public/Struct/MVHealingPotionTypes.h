#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVHealingPotionTypes.generated.h"

/**
 * 플레이어 회복약 슬롯의 런타임 상태.
 *
 * 회복약 서브모듈이 사용 설정과 인벤토리 수량을 합쳐 HUD에 전달한다.
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
