#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MVInventoryComponent.generated.h"

class UMVPlayerConsumable;
class UMVWeaponComponent;
struct FMVInventorySaveData;
struct FMVWeaponProgressSaveData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnInventoryChanged, FGameplayTag, ItemTag);

/**
 * 캐릭터의 소유 아이템과 수량을 태그로 관리한다.
 * 테이블 정의와 보유 한도를 검사하고, NPC 무기에는 캐릭터별 무기 행을 허용한다.
 * 저장 복원은 수량을 일괄 교체한다. 일반 변경은 장착과 저장 자료를 일치시킨 뒤 알린다.
 */
UCLASS(BlueprintType, ClassGroup = (Maverick))
class MAVERICK_API UMVInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Inventory")
	bool TryAddItem(FGameplayTag ItemTag, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Inventory")
	bool TryRemoveItem(FGameplayTag ItemTag, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Maverick|Inventory")
	int32 GetItemCount(FGameplayTag ItemTag) const;

	const TMap<FGameplayTag, int32>& GetItems() const { return Items; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Inventory")
	FMVOnInventoryChanged OnInventoryChanged;

private:
	friend class UMVPlayerConsumable;
	friend class UMVWeaponComponent;

	bool TryAddWeapon(const FDataTableRowHandle& Row, bool bNotify = true);
	FDataTableRowHandle FindOwnedWeaponRow(FGameplayTag ItemTag) const;
	FDataTableRowHandle FindWeaponDefinition(FGameplayTag ItemTag) const;
	bool IsWeaponItem(FGameplayTag ItemTag) const;
	bool SetPotionCount(FGameplayTag ItemTag, int32 Quantity);
	void UpdateItemCount(FGameplayTag ItemTag, int32 Quantity, bool bNotify = true);
	void RestorePlayerItems(const FMVInventorySaveData& Inventory, const FMVWeaponProgressSaveData& Weapons);

	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> Items;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FDataTableRowHandle> LocalWeaponDefinitions;

	TWeakObjectPtr<UMVWeaponComponent> WeaponOwner;
};
