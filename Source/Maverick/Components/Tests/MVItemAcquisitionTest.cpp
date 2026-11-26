#if WITH_DEV_AUTOMATION_TESTS

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVInventoryComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tables/MVTableManager.h"
#include "Tags/MVGameplayTags.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVItemAcquisitionTest, "Maverick.Inventory.Acquisition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVItemAcquisitionTest::RunTest(const FString& Parameters)
{
	const FGameplayTag Material = MVGameplayTags::Item_Material_IronShard;
	const FGameplayTag Weapon = MVGameplayTags::Item_Weapon_DualSword_TestSword;
	const FGameplayTag Potion = MVGameplayTags::Item_HealingPotion_Basic;
	const UMVTableManager* Tables = UMVTableManager::Get();
	const FMVInventoryItemDefinition* WeaponDefinition = Tables ? Tables->FindInventoryItemDefinition(Weapon) : nullptr;
	const FMVInventoryItemDefinition* PotionDefinition = Tables ? Tables->FindInventoryItemDefinition(Potion) : nullptr;
	if (!TestNotNull(TEXT("Player weapon definition"), WeaponDefinition)
		|| !TestNotNull(TEXT("Potion definition"), PotionDefinition))
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	ON_SCOPE_EXIT { World->DestroyWorld(false); };
	AMVPlayerCharacter* Player = World->SpawnActor<AMVPlayerCharacter>();
	if (!TestNotNull(TEXT("Test player"), Player))
	{
		return false;
	}

	UMVInventoryComponent* Inventory = Player->InventoryComponent;
	UMVWeaponComponent* Weapons = Player->WeaponComponent;
	// 자동 초기화가 기본 저장 슬롯을 읽지 않도록 시험 슬롯을 직접 연결한다.
	auto* State = NewObject<UMVWorldStateSubsystem>(NewObject<UGameInstance>());
	State->ResetSaveData();
	const FString Slot = TEXT("CodexItemAcquisition_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	State->ActiveSaveSlotName = Slot;
	ON_SCOPE_EXIT
	{
		if (State->DoesSaveSlotExist(Slot, 0))
		{
			TestTrue(TEXT("Remove created test slot"), State->DeleteSaveSlot(Slot, 0));
		}
	};
	Inventory->WeaponOwner = Weapons;
	Weapons->Inventory = Inventory;
	Weapons->WeaponSaveState = State;

	TArray<FMVItemAcquisition> Events;
	Inventory->OnItemAcquired.AddLambda([&Events](const FMVItemAcquisition& Data) { Events.Add(Data); });

	const FName Field = TEXT("AcquisitionTestField");
	const FName Pickup = TEXT("AcquisitionTestPickup");
	TestTrue(TEXT("Bad placed pickup identity is rejected"),
		Inventory->TryGrantPlacedPickup(Material, 3, NAME_None, Pickup) == EMVInventoryGrantResult::PlacedPickupUnavailable);
	TestEqual(TEXT("Rejected commit restores count"), Inventory->GetItemCount(Material), 0);
	TestFalse(TEXT("Rejected commit restores discovery"), Inventory->BuildSaveData().DiscoveredItems.Contains(Material));
	TestEqual(TEXT("Rejected commit emits no acquisition"), Events.Num(), 0);

	TestTrue(TEXT("Placed pickup grants the full bundle"),
		Inventory->TryGrantPlacedPickup(Material, 3, Field, Pickup) == EMVInventoryGrantResult::Added);
	TestTrue(TEXT("Placed pickup is consumed"), State->IsOneTimeSpawnConsumed(Field, Pickup));
	TestTrue(TEXT("Placed pickup writes its discovery"), State->GetSaveData().Inventory.DiscoveredItems.Contains(Material));
	TestTrue(TEXT("Repeat pickup ID is rejected"),
		Inventory->TryGrantPlacedPickup(Material, 1, Field, Pickup) == EMVInventoryGrantResult::PlacedPickupUnavailable);
	TestEqual(TEXT("Only one placed pickup event"), Events.Num(), 1);
	if (Events.Num() == 1)
	{
		TestEqual(TEXT("Acquisition tag"), Events[0].ItemTag, Material);
		TestEqual(TEXT("Acquisition quantity"), Events[0].GrantedQuantity, 3);
		TestTrue(TEXT("First acquisition"), Events[0].bFirstAcquisition);
	}

	TestTrue(TEXT("Blueprint grant entry succeeds"), Inventory->TryAddItem(Material, 2));
	TestTrue(TEXT("Repeated acquisition is recognized"), Events.Num() == 2 && !Events.Last().bFirstAcquisition
		&& Events.Last().GrantedQuantity == 2);
	TestTrue(TEXT("Discard all material"), Inventory->TryRemoveItem(Material, 5));
	TestFalse(TEXT("Invalid tag cannot be granted"), Inventory->TryAddItem(FGameplayTag(), 1));
	TestFalse(TEXT("Zero cannot be granted"), Inventory->TryAddItem(Material, 0));
	TestTrue(TEXT("Normal weapon grant"), Weapons->AcquireWeapon(WeaponDefinition->Row));
	TestFalse(TEXT("Duplicate weapon cannot be granted"), Weapons->AcquireWeapon(WeaponDefinition->Row));
	TestTrue(TEXT("Weapon grant sends one new event"), Events.Num() == 3 && Events.Last().ItemTag == Weapon
		&& Events.Last().bFirstAcquisition);

	TestTrue(TEXT("Refill potion count"), Inventory->SetPotionCount(Potion, PotionDefinition->StackLimit));
	TestTrue(TEXT("Full pouch rejects an extra potion"),
		Inventory->TryGrantItem(Potion, 1) == EMVInventoryGrantResult::CarryLimit);
	TestEqual(TEXT("Refill, remove and failed grants sent no events"), Events.Num(), 3);
	TestTrue(TEXT("Ordinary save retains previously discovered material"), State->SaveActiveSlot());
	auto* Reloaded = NewObject<UMVWorldStateSubsystem>(NewObject<UGameInstance>());
	if (!TestTrue(TEXT("Fresh save subsystem reloads test slot"), Reloaded->LoadFromSlot(Slot, 0)))
	{
		return false;
	}

	const FMVInventorySaveData& SavedItems = Reloaded->GetSaveData().Inventory;
	TestEqual(TEXT("Discarded material count is absent on disk"), SavedItems.ItemCounts.FindRef(Material), 0);
	TestTrue(TEXT("Discovery survives disk round trip"), SavedItems.DiscoveredItems.Contains(Material));
	Inventory->RestorePlayerItems(SavedItems, Reloaded->GetWeaponProgress());
	TestEqual(TEXT("Restore emits no acquisition"), Events.Num(), 3);
	TestTrue(TEXT("Reacquisition after reload succeeds"), Inventory->TryAddItem(Material));
	TestTrue(TEXT("Reacquisition survives an empty saved stack"), Events.Num() == 4 && !Events.Last().bFirstAcquisition);

	FMVInventorySaveData LegacyItems;
	LegacyItems.bHasRecord = true;
	LegacyItems.ItemCounts.Add(Material, 1);
	Inventory->RestorePlayerItems(LegacyItems, FMVWeaponProgressSaveData());
	TestEqual(TEXT("Legacy restore emits no acquisition"), Events.Num(), 4);
	TestTrue(TEXT("Old save already knows its held item"), Inventory->BuildSaveData().DiscoveredItems.Contains(Material));
	TestFalse(TEXT("Restore clears history from the prior slot"), Inventory->BuildSaveData().DiscoveredItems.Contains(Weapon));

	TestTrue(TEXT("Default weapon is initialized silently"), Inventory->TryAddWeapon(WeaponDefinition->Row, false));
	TestTrue(TEXT("Default weapon is recorded as known"), Inventory->BuildSaveData().DiscoveredItems.Contains(Weapon));
	TestEqual(TEXT("Default weapon emitted no acquisition"), Events.Num(), 4);

	AMVCharacterBase* NPC = World->SpawnActor<AMVCharacterBase>(FVector(1000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("Test non-player character"), NPC))
	{
		UMVInventoryComponent* NPCInventory = NPC->InventoryComponent;
		UMVWeaponComponent* NPCWeapons = NPC->WeaponComponent;
		NPCInventory->WeaponOwner = NPCWeapons;
		NPCWeapons->Inventory = NPCInventory;
		int32 NPCEvents = 0;
		const FDelegateHandle NPCEvent = NPCInventory->OnItemAcquired.AddLambda([&](const auto&) { ++NPCEvents; });
		TestTrue(TEXT("Non-player can receive its weapon"), NPCWeapons->AcquireWeapon(WeaponDefinition->Row));
		TestEqual(TEXT("Non-player weapon grant emitted no acquisition"), NPCEvents, 0);
		NPCInventory->OnItemAcquired.Remove(NPCEvent);
	}
	return !HasAnyErrors();
}

#endif
