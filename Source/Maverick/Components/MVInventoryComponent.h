#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "MVInventoryComponent.generated.h"

class UMVPlayerConsumable;
class UMVWeaponComponent;
class UMVWorldStateSubsystem;
struct FMVInventorySaveData;
struct FMVWeaponProgressSaveData;

/** 드랍과 같은 호출부가 지급 실패 이유를 안내하는 데 사용하는 결과. */
enum class EMVInventoryGrantResult : uint8
{
	Added,
	InvalidItemOrQuantity,
	CarryLimit,
	WeaponAlreadyOwned,
	PlacedPickupUnavailable
};

/** 지급이 실제로 확정된 아이템과 수량을 전달한다. */
struct FMVItemAcquisition
{
	FGameplayTag ItemTag;
	int32 GrantedQuantity = 0;
	bool bFirstAcquisition = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnInventoryChanged, FGameplayTag, ItemTag);
DECLARE_MULTICAST_DELEGATE_OneParam(FMVOnItemAcquired, const FMVItemAcquisition&);

/**
 * 캐릭터의 소유 아이템과 수량을 태그로 관리한다.
 * 테이블 정의와 보유 한도를 검사하고, NPC 무기에는 캐릭터별 무기 행을 허용한다.
 * 저장 복원은 소지 수량과 발견 기록을 교체한다. 실제 지급에 성공한 경우에만 습득 신호를 보낸다.
 */
UCLASS(BlueprintType, ClassGroup = (Maverick))
class MAVERICK_API UMVInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Inventory")
	bool TryAddItem(FGameplayTag ItemTag, int32 Quantity = 1);
	EMVInventoryGrantResult TryGrantItem(FGameplayTag ItemTag, int32 Quantity);
	EMVInventoryGrantResult TryGrantPlacedPickup(FGameplayTag ItemTag, int32 Quantity, FName FieldId, FName PickupId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Inventory")
	bool TryRemoveItem(FGameplayTag ItemTag, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Maverick|Inventory")
	int32 GetItemCount(FGameplayTag ItemTag) const;

	const TMap<FGameplayTag, int32>& GetItems() const { return Items; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Inventory")
	FMVOnInventoryChanged OnInventoryChanged;

	FMVOnItemAcquired OnItemAcquired;

private:
	friend class UMVPlayerConsumable;
	friend class UMVWeaponComponent;

	bool TryAddWeapon(const FDataTableRowHandle& Row, bool bNotify = true);
	FDataTableRowHandle FindOwnedWeaponRow(FGameplayTag ItemTag) const;
	FDataTableRowHandle FindWeaponDefinition(FGameplayTag ItemTag) const;
	bool IsWeaponItem(FGameplayTag ItemTag) const;
	bool SetPotionCount(FGameplayTag ItemTag, int32 Quantity);
	EMVInventoryGrantResult CheckGrantItem(FGameplayTag ItemTag, int32 Quantity) const;
	void UpdateItemCount(FGameplayTag ItemTag, int32 Quantity, bool bNotify = true);
	void NotifyCommittedAcquisition(const FMVItemAcquisition& Acquisition);
	FMVInventorySaveData BuildSaveData() const;
	void RestorePlayerItems(const FMVInventorySaveData& Inventory, const FMVWeaponProgressSaveData& Weapons);

	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> Items;

	TSet<FGameplayTag> DiscoveredItems;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FDataTableRowHandle> LocalWeaponDefinitions;

	TWeakObjectPtr<UMVWeaponComponent> WeaponOwner;
};
