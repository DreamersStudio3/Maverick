#include "Tables/MVTableManager.h"

#include "Tables/MVHealingPotionTableTypes.h"
#include "Tables/MVItemTableTypes.h"
#include "Tables/MVWeaponTableTypes.h"
#include "Tags/MVGameplayTags.h"

const FMVInventoryItemDefinition* UMVTableManager::FindInventoryItemDefinition(const FGameplayTag ItemTag) const
{
	return InventoryItemIndex.Find(ItemTag);
}

void UMVTableManager::RebuildInventoryItemIndex()
{
	const UDataTable* Groups = FindDataTable(TEXT("InventoryGroups"));
	if (!Groups || Groups->GetRowStruct() != FMVInventoryGroupTableRow::StaticStruct())
	{
		AddLoadError(TEXT("[Inventory] Expected InventoryGroups table with MVInventoryGroupTableRow rows."));
		Groups = nullptr;
	}

	const struct
	{
		FName TableName;
		const UScriptStruct* RowType;
		FName DefaultGroup;
		EMVInventoryGroupCategory DefaultGroupCategory;
		EMVItemRarity DefaultRarity;
	} ItemTables[] = {
		{ TEXT("Weapons_Player"), FMVWeaponTableRow::StaticStruct(),
			TEXT("Weapons.Other"), EMVInventoryGroupCategory::Weapons, EMVItemRarity::Epic },
		{ TEXT("Items_Consumable"), FMVHealingPotionTableRow::StaticStruct(),
			TEXT("Consumables.Potion"), EMVInventoryGroupCategory::Consumables, EMVItemRarity::Common },
		{ TEXT("Items_Materials"), FMVItemTableRow::StaticStruct(),
			TEXT("Materials.General"), EMVInventoryGroupCategory::Materials, EMVItemRarity::Rare }
	};

	TSet<FGameplayTag> DuplicateTags;
	for (const auto& Entry : ItemTables)
	{
		const UDataTable* Table = FindDataTable(Entry.TableName);
		if (!Table || Table->GetRowStruct() != Entry.RowType)
		{
			AddLoadError(FString::Printf(TEXT("[Inventory] Expected %s table with %s rows."),
				*Entry.TableName.ToString(), *Entry.RowType->GetName()));
			continue;
		}

		const bool bWeaponTable = Entry.RowType == FMVWeaponTableRow::StaticStruct();
		const bool bPotionTable = Entry.RowType == FMVHealingPotionTableRow::StaticStruct();
		const auto* DefaultGroup = Groups
			? Groups->FindRow<FMVInventoryGroupTableRow>(Entry.DefaultGroup, TEXT("Inventory default group"), false)
			: nullptr;
		if (Groups && !DefaultGroup)
		{
			AddLoadError(FString::Printf(TEXT("[Inventory] Default group %s for %s is missing."),
				*Entry.DefaultGroup.ToString(), *Entry.TableName.ToString()));
		}

		for (const auto& Pair : Table->GetRowMap())
		{
			const auto* Row = reinterpret_cast<const FMVItemTableRow*>(Pair.Value);
			if (Row && Row->ItemTag.MatchesTag(MVGameplayTags::Item_Weapon_BareHand))
			{
				continue;
			}

			const bool bExpectedType = Row && (bWeaponTable ? Row->ItemType == EMVItemType::Equipment
				: bPotionTable ? Row->ItemType == EMVItemType::HealingPotion
				: Row->ItemType != EMVItemType::None && Row->ItemType != EMVItemType::HealingPotion);
			if (!bExpectedType || !Row->ItemTag.IsValid())
			{
				AddLoadError(FString::Printf(TEXT("[Inventory] Invalid item tag or type in %s:%s."),
					*Table->GetName(), *Pair.Key.ToString()));
				continue;
			}

			if (const FMVInventoryItemDefinition* Previous = InventoryItemIndex.Find(Row->ItemTag))
			{
				AddLoadError(FString::Printf(TEXT("[Inventory] Duplicate tag %s in %s:%s and %s:%s."),
					*Row->ItemTag.ToString(), *Table->GetName(), *Pair.Key.ToString(),
					*GetNameSafe(Previous->Row.DataTable), *Previous->Row.RowName.ToString()));
				InventoryItemIndex.Remove(Row->ItemTag);
				DuplicateTags.Add(Row->ItemTag);
				continue;
			}
			if (DuplicateTags.Contains(Row->ItemTag))
			{
				AddLoadError(FString::Printf(TEXT("[Inventory] Duplicate tag %s in %s:%s."),
					*Row->ItemTag.ToString(), *Table->GetName(), *Pair.Key.ToString()));
				continue;
			}

			FMVInventoryItemDefinition& Definition = InventoryItemIndex.Add(Row->ItemTag);
			Definition.Row.DataTable = Table;
			Definition.Row.RowName = Pair.Key;
			Definition.Rarity = Row->Rarity == EMVItemRarity::UseItemType ? Entry.DefaultRarity : Row->Rarity;
			Definition.InventoryGroupId = Entry.DefaultGroup;
			Definition.InventorySortOrder = Row->InventorySortOrder;
			const auto* Group = DefaultGroup;
			if (Groups && !Row->InventoryGroupId.IsNone())
			{
				if (const auto* RequestedGroup = Groups->FindRow<FMVInventoryGroupTableRow>(
					Row->InventoryGroupId, TEXT("Inventory item group"), false))
				{
					Definition.InventoryGroupId = Row->InventoryGroupId;
					Group = RequestedGroup;
				}
				else
				{
					AddLoadError(FString::Printf(TEXT("[Inventory] Unknown group %s in %s:%s. Using %s."),
						*Row->InventoryGroupId.ToString(), *Table->GetName(), *Pair.Key.ToString(),
						*Entry.DefaultGroup.ToString()));
				}
			}
			Definition.InventoryGroupCategory = Group ? Group->Category : Entry.DefaultGroupCategory;
			Definition.InventoryGroupSortOrder = Group ? Group->SortOrder : 0;

			if (bWeaponTable)
			{
				Definition.StackLimit = 1;
			}
			else if (bPotionTable)
			{
				Definition.StackLimit = FMath::Max(0, static_cast<const FMVHealingPotionTableRow*>(Row)->MaxCarryCount);
			}
		}
	}
}
