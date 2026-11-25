#include "Components/MVInventoryComponent.h"

#include "Character/PC/Consumable/MVPlayerConsumable.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVWeaponComponent.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/MVWorldStateTypes.h"
#include "Tables/MVHealingPotionTableTypes.h"
#include "Tables/MVTableManager.h"
#include "Tables/MVWeaponTableTypes.h"
#include "Tags/MVGameplayTags.h"

namespace
{
bool MVInventoryIsHealingPotionDefinition(const FMVInventoryItemDefinition* Definition)
{
	return Definition && Definition->Row.DataTable
		&& Definition->Row.DataTable->GetRowStruct() == FMVHealingPotionTableRow::StaticStruct();
}
}

#if !UE_BUILD_SHIPPING
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

namespace
{
FAutoConsoleCommandWithWorldAndArgs MVInventoryGrantConsoleCommand(
	TEXT("MV.Inventory.Grant"), TEXT("Grant a defined item to the local player. Usage: MV.Inventory.Grant <ItemTag> [quantity]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		int32 Quantity = 1;
		if (Args.IsEmpty() || Args.Num() > 2 || (Args.Num() == 2 && !LexTryParseString(Quantity, *Args[1])))
		{
			UE_LOG(LogTemp, Warning, TEXT("Usage: MV.Inventory.Grant <ItemTag> [quantity]"));
			return;
		}
		const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		auto* Player = Controller && Controller->IsLocalController()
			? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		const bool bGranted = Player && Player->InventoryComponent && Player->InventoryComponent->TryAddItem(Tag, Quantity);
		UE_LOG(LogTemp, Display, TEXT("[Inventory] %s %s x%d."), bGranted ? TEXT("Granted") : TEXT("Rejected"), *Args[0], Quantity);
	}));
}
#endif

bool UMVInventoryComponent::TryAddItem(const FGameplayTag ItemTag, const int32 Quantity)
{
	return TryGrantItem(ItemTag, Quantity) == EMVInventoryGrantResult::Added;
}

EMVInventoryGrantResult UMVInventoryComponent::TryGrantItem(const FGameplayTag ItemTag, const int32 Quantity)
{
	const EMVInventoryGrantResult Result = CheckGrantItem(ItemTag, Quantity);
	if (Result == EMVInventoryGrantResult::Added)
	{
		const FMVItemAcquisition Acquisition{ItemTag, Quantity, !DiscoveredItems.Contains(ItemTag)};
		UpdateItemCount(ItemTag, GetItemCount(ItemTag) + Quantity);
		OnItemAcquired.Broadcast(Acquisition);
	}
	return Result;
}

EMVInventoryGrantResult UMVInventoryComponent::TryGrantPlacedPickup(
	const FGameplayTag ItemTag, const int32 Quantity, const FName FieldId, const FName PickupId)
{
	const EMVInventoryGrantResult Result = CheckGrantItem(ItemTag, Quantity);
	if (Result != EMVInventoryGrantResult::Added)
	{
		return Result;
	}

	UMVWeaponComponent* Weapons = WeaponOwner.Get();
	UMVWorldStateSubsystem* State = Weapons ? Weapons->WeaponSaveState.Get() : nullptr;
	if (!State || Weapons->bRestoringWeaponProgress)
	{
		return EMVInventoryGrantResult::PlacedPickupUnavailable;
	}

	const int32 PreviousQuantity = GetItemCount(ItemTag);
	const FMVItemAcquisition Acquisition{ItemTag, Quantity, !DiscoveredItems.Contains(ItemTag)};
	UpdateItemCount(ItemTag, PreviousQuantity + Quantity, false);
	if (!State->TryCommitPlacedPickup(FieldId, PickupId, BuildSaveData()))
	{
		UpdateItemCount(ItemTag, PreviousQuantity, false);
		if (Acquisition.bFirstAcquisition)
		{
			DiscoveredItems.Remove(ItemTag);
		}
		return EMVInventoryGrantResult::PlacedPickupUnavailable;
	}

	NotifyCommittedAcquisition(Acquisition);
	return EMVInventoryGrantResult::Added;
}

void UMVInventoryComponent::NotifyCommittedAcquisition(const FMVItemAcquisition& Acquisition)
{
	if (UMVWeaponComponent* Weapons = WeaponOwner.Get(); Weapons && IsWeaponItem(Acquisition.ItemTag))
	{
		Weapons->OnOwnedWeaponsChanged.Broadcast();
	}
	OnInventoryChanged.Broadcast(Acquisition.ItemTag);
	OnItemAcquired.Broadcast(Acquisition);
}

EMVInventoryGrantResult UMVInventoryComponent::CheckGrantItem(const FGameplayTag ItemTag, const int32 Quantity) const
{
	const UMVTableManager* Tables = UMVTableManager::Get(this);
	const FMVInventoryItemDefinition* Definition = Tables ? Tables->FindInventoryItemDefinition(ItemTag) : nullptr;
	if (!Definition || Quantity <= 0)
	{
		return EMVInventoryGrantResult::InvalidItemOrQuantity;
	}

	const int32 Current = GetItemCount(ItemTag);
	if (Quantity > Definition->StackLimit - Current)
	{
		const bool bAlreadyOwnedWeapon = Current > 0 && IsWeaponItem(ItemTag);
		return bAlreadyOwnedWeapon ? EMVInventoryGrantResult::WeaponAlreadyOwned : EMVInventoryGrantResult::CarryLimit;
	}

	return EMVInventoryGrantResult::Added;
}

bool UMVInventoryComponent::TryRemoveItem(const FGameplayTag ItemTag, const int32 Quantity)
{
	const int32 Current = GetItemCount(ItemTag);
	if (Quantity <= 0 || Quantity > Current)
	{
		return false;
	}

	if (const UMVWeaponComponent* Weapons = WeaponOwner.Get();
		Weapons && IsWeaponItem(ItemTag) && !Weapons->CanRemoveOwnedWeapon())
	{
		return false;
	}

	UpdateItemCount(ItemTag, Current - Quantity);
	return true;
}

int32 UMVInventoryComponent::GetItemCount(const FGameplayTag ItemTag) const
{
	return Items.FindRef(ItemTag);
}

bool UMVInventoryComponent::TryAddWeapon(const FDataTableRowHandle& Handle, const bool bNotify)
{
	const auto* Row = Handle.GetRow<FMVWeaponTableRow>(TEXT("Inventory weapon grant"));
	if (!Row || !Row->ItemTag.IsValid() || Row->ItemTag.MatchesTag(MVGameplayTags::Item_Weapon_BareHand)
		|| Items.Contains(Row->ItemTag))
	{
		return false;
	}

	if (Cast<AMVPlayerCharacter>(GetOwner()))
	{
		if (FindWeaponDefinition(Row->ItemTag) != Handle)
		{
			return false;
		}
		if (bNotify)
		{
			return TryGrantItem(Row->ItemTag, 1) == EMVInventoryGrantResult::Added;
		}
	}
	else
	{
		LocalWeaponDefinitions.Add(Row->ItemTag, Handle);
	}

	UpdateItemCount(Row->ItemTag, 1, bNotify);
	return true;
}

FDataTableRowHandle UMVInventoryComponent::FindOwnedWeaponRow(const FGameplayTag ItemTag) const
{
	return Items.Contains(ItemTag) ? FindWeaponDefinition(ItemTag) : FDataTableRowHandle();
}

FDataTableRowHandle UMVInventoryComponent::FindWeaponDefinition(const FGameplayTag ItemTag) const
{
	if (const FDataTableRowHandle* Row = LocalWeaponDefinitions.Find(ItemTag))
	{
		return *Row;
	}
	const UMVTableManager* Tables = UMVTableManager::Get(this);
	const FMVInventoryItemDefinition* Definition = Tables ? Tables->FindInventoryItemDefinition(ItemTag) : nullptr;
	return Definition && Definition->Row.DataTable && Definition->Row.DataTable->GetRowStruct() == FMVWeaponTableRow::StaticStruct()
		? Definition->Row : FDataTableRowHandle();
}

bool UMVInventoryComponent::IsWeaponItem(const FGameplayTag ItemTag) const
{
	const UMVWeaponComponent* Weapons = WeaponOwner.Get();
	return !FindWeaponDefinition(ItemTag).IsNull()
		|| (Weapons && (Weapons->FindQuickSlotIndexForItemTag(ItemTag) != INDEX_NONE
			|| Weapons->GetEquippedWeaponState().ItemTag == ItemTag));
}

bool UMVInventoryComponent::SetPotionCount(const FGameplayTag ItemTag, const int32 Quantity)
{
	const UMVTableManager* Tables = UMVTableManager::Get(this);
	const FMVInventoryItemDefinition* Definition = Tables ? Tables->FindInventoryItemDefinition(ItemTag) : nullptr;
	if (!MVInventoryIsHealingPotionDefinition(Definition)
		|| Quantity < 0 || Quantity > Definition->StackLimit || Quantity == GetItemCount(ItemTag))
	{
		return false;
	}

	UpdateItemCount(ItemTag, Quantity);
	return true;
}

void UMVInventoryComponent::UpdateItemCount(const FGameplayTag ItemTag, const int32 Quantity, const bool bNotify)
{
	const bool bWeapon = bNotify && IsWeaponItem(ItemTag);
	if (Quantity == 0)
	{
		Items.Remove(ItemTag);
	}
	else
	{
		Items.Add(ItemTag, Quantity);
		DiscoveredItems.Add(ItemTag);
	}
	if (bNotify)
	{
		if (UMVWeaponComponent* Weapons = WeaponOwner.Get())
		{
			Weapons->HandleInventoryItemChanged(ItemTag, bWeapon);
		}
		OnInventoryChanged.Broadcast(ItemTag);
	}
}

FMVInventorySaveData UMVInventoryComponent::BuildSaveData() const
{
	FMVInventorySaveData Snapshot;
	Snapshot.bHasRecord = true;
	Snapshot.ItemCounts = Items;
	Snapshot.DiscoveredItems = DiscoveredItems;
	return Snapshot;
}

void UMVInventoryComponent::RestorePlayerItems(const FMVInventorySaveData& Inventory, const FMVWeaponProgressSaveData& Weapons)
{
	Items.Reset();
	DiscoveredItems = Inventory.DiscoveredItems;
	const UMVTableManager* Tables = UMVTableManager::Get(this);
	if (Inventory.bHasRecord)
	{
		for (const auto& Pair : Inventory.ItemCounts)
		{
			const FMVInventoryItemDefinition* Definition = Tables ? Tables->FindInventoryItemDefinition(Pair.Key) : nullptr;
			const int32 Limit = LocalWeaponDefinitions.Contains(Pair.Key) ? 1 : Definition ? Definition->StackLimit : 0;
			if (Pair.Value < 1 || Pair.Value > Limit)
			{
				UE_LOG(LogTemp, Warning, TEXT("Inventory restore: invalid count %d for %s (limit %d)."),
					Pair.Value, *Pair.Key.ToString(), Limit);
			}
			if (Pair.Value > 0 && Limit > 0)
			{
				UpdateItemCount(Pair.Key, FMath::Min(Pair.Value, Limit), false);
			}
		}
		return;
	}

	if (Weapons.bHasRecord)
	{
		for (const auto& Pair : Weapons.OwnedWeapons)
		{
			const FDataTableRowHandle Definition = FindWeaponDefinition(Pair.Key);
			if (!Definition.IsNull() && Definition.RowName == Pair.Value.RowName
				&& Definition.DataTable == Pair.Value.Table.LoadSynchronous())
			{
				UpdateItemCount(Pair.Key, 1, false);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Inventory migration: unavailable weapon %s from %s:%s."),
					*Pair.Key.ToString(), *Pair.Value.Table.ToString(), *Pair.Value.RowName.ToString());
			}
		}
	}

	const auto* Player = Cast<AMVPlayerCharacter>(GetOwner());
	FGameplayTag PotionTag;
	int32 PotionCount = 0;
	if (Player && Player->PlayerConsumable)
	{
		Player->PlayerConsumable->GetDefaultHealingPotionStack(PotionTag, PotionCount);
	}
	if (const FMVInventoryItemDefinition* Potion = Tables ? Tables->FindInventoryItemDefinition(PotionTag) : nullptr;
		MVInventoryIsHealingPotionDefinition(Potion) && PotionCount > 0 && Potion->StackLimit > 0)
	{
		UpdateItemCount(PotionTag, FMath::Min(PotionCount, Potion->StackLimit), false);
	}
}
