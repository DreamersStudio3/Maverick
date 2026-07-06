#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Tables/MVItemTableTypes.h"
#include "MVHealingPotionTableTypes.generated.h"

/**
 * 전투 프로토타입용 회복약 row.
 *
 * 이번 범위는 HP 회복약 한 종류의 vertical slice에 한정한다. 넓은 Consumable 공통 row가 아니라,
 * 회복량과 기본 보유량, 사용 액션만 가진 좁은 테이블로 시작한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHealingPotionTableRow : public FMVItemTableRow
{
	GENERATED_BODY()

	FMVHealingPotionTableRow()
	{
		ItemType = EMVItemType::HealingPotion;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|HealingPotion", meta = (ClampMin = "0.0"))
	float HealAmount = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|HealingPotion", meta = (ClampMin = "0"))
	int32 DefaultCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|HealingPotion", meta = (ClampMin = "0"))
	int32 MaxCarryCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|HealingPotion|Action")
	FDataTableRowHandle UseActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|HealingPotion|Action")
	FName UseActionStartSection = NAME_None;
};
