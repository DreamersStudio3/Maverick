#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MVEnemyDropTableTypes.generated.h"

class AMVItemPickup;

UENUM(BlueprintType)
enum class EMVEnemyRewardDelivery : uint8
{
	WorldPickup,
	DirectToInventory
};

/** 적이 죽을 때 확정으로 주는 아이템 한 종류와 수량. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVGuaranteedEnemyDrop
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Enemy Drop", meta = (Categories = "Item"))
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, Category = "Enemy Drop", meta = (ClampMin = "1"))
	int32 Quantity = 1;
};

/** 적 생성 지점들이 공유하는 사망 보상. 기존 행은 바닥 드롭으로 해석한다. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVEnemyDropTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Enemy Drop")
	EMVEnemyRewardDelivery Delivery = EMVEnemyRewardDelivery::WorldPickup;

	UPROPERTY(EditAnywhere, Category = "Enemy Drop", meta = (EditCondition = "Delivery == EMVEnemyRewardDelivery::WorldPickup"))
	TSubclassOf<AMVItemPickup> PickupClass;

	UPROPERTY(EditAnywhere, Category = "Enemy Drop")
	TArray<FMVGuaranteedEnemyDrop> GuaranteedDrops;
};
