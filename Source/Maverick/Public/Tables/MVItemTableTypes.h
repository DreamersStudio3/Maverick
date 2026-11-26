#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Enum/MVItemEnums.h"
#include "GameplayTagContainer.h"
#include "Tables/MVTableTypes.h"
#include "MVItemTableTypes.generated.h"

/**
 * 아이템 테이블 row의 공통 표시 데이터.
 *
 * ItemTag는 게임플레이에서 사용하는 canonical key이며 별도 ItemId를 두지 않는다.
 * DataTable row name은 에디터/테이블 조회용 locator로만 취급한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVItemTableRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item", meta = (Categories = "Item"))
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item")
	EMVItemType ItemType = EMVItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item")
	EMVItemRarity Rarity = EMVItemRarity::UseItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Inventory")
	FName InventoryGroupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Inventory")
	int32 InventorySortOrder = 0;
};

/** DT_InventoryGroups의 행 이름으로 연결하는 목록 표시 분류와 정렬 순서. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVInventoryGroupTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Inventory")
	EMVInventoryGroupCategory Category = EMVInventoryGroupCategory::Other;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Inventory")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Inventory")
	int32 SortOrder = 0;
};
