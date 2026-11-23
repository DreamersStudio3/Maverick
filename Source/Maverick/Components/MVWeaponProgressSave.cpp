#include "Components/MVWeaponComponent.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVInventoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "System/MVWorldStateSubsystem.h"
#include "Weapon/Skill/MVSkillSelectionData.h"
#include "Weapon/Skill/MVSkillExecutionData.h"
#include "Public/Tables/MVSkillDataTableColumn.h"

namespace
{
FMVSavedWeaponRow MVWeaponProgressMakeRowKey(const FDataTableRowHandle& Row)
{
	FMVSavedWeaponRow Key;
	Key.Table = TSoftObjectPtr<UDataTable>(FSoftObjectPath(Row.DataTable.Get()));
	Key.RowName = Row.RowName;
	return Key;
}

FDataTableRowHandle MVWeaponProgressResolveRow(const FMVSavedWeaponRow& Key)
{
	FDataTableRowHandle Row;
	UDataTable* Table = Key.RowName.IsNone() ? nullptr : Key.Table.LoadSynchronous();
	if (Table && Table->GetRowMap().Contains(Key.RowName))
	{
		Row.DataTable = Table;
		Row.RowName = Key.RowName;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon progress: unavailable saved row %s:%s."),
			*Key.Table.ToString(), *Key.RowName.ToString());
	}
	return Row;
}
}

void UMVWeaponComponent::InitializeWeaponProgress()
{
	const UWorld* World = GetWorld();
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	if (World && World->GetNetMode() == NM_Standalone && Cast<AMVPlayerCharacter>(GetOwner())
		&& (!Controller || !Controller->GetPawn() || Controller->GetPawn() == GetOwner()))
	{
		WeaponSaveState = UMVWorldStateSubsystem::Get(this);
	}

	if (UMVWorldStateSubsystem* State = WeaponSaveState.Get())
	{
		State->OnWorldStateLoaded.AddUniqueDynamic(this, &ThisClass::HandleWeaponSaveLoaded);
		HandleWeaponSaveLoaded(State->ActiveSaveSlotName);
	}
	else
	{
		RestoreWeaponProgress(FMVWeaponProgressSaveData(), FMVInventorySaveData());
		NotifyWeaponProgressRestored();
	}
}

void UMVWeaponComponent::HandleWeaponSaveLoaded(FString SlotName)
{
	if (UMVWorldStateSubsystem* State = WeaponSaveState.Get())
	{
		const FMVWeaponProgressSaveData Progress = State->GetWeaponProgress();
		const FMVInventorySaveData SavedItems = State->GetSaveData().Inventory;
		RestoreWeaponProgress(Progress, SavedItems);
		if (!Progress.bHasRecord || !SavedItems.bHasRecord
			|| (Inventory && !SavedItems.ItemCounts.OrderIndependentCompareEqual(Inventory->GetItems()))
			|| Progress.QuickSlots != QuickSlotWeaponTags || Progress.ActiveSlot != ActiveQuickSlotIndex
			|| Progress.EquippedWeapon != CurrentWeaponState.ItemTag)
		{
			StoreWeaponProgress();
		}
		NotifyWeaponProgressRestored();
	}
}

void UMVWeaponComponent::RestoreWeaponProgress(
	const FMVWeaponProgressSaveData& Progress, const FMVInventorySaveData& InventoryProgress)
{
	TGuardValue<bool> RestoreGuard(bRestoringWeaponProgress, true);
	if (Inventory)
	{
		Inventory->RestorePlayerItems(InventoryProgress, Progress);
	}
	QuickSlotWeaponTags.Reset();
	EnsureQuickSlotStorage();
	ActiveQuickSlotIndex = INDEX_NONE;
	CurrentWeaponState.bValid = false;
	SelectedPassiveByWeaponType.Reset();
	CurrentSkillPoints = 0;
	for (auto& Style : SkillTreeStates)
	{
		Style.Value.UnlockedSkills.Reset();
		Style.Value.EquippedSkill.Reset();
	}
	InitSkillTreeStates();

	if (!Progress.bHasRecord)
	{
		if (InventoryProgress.bHasRecord || !TryInitializeDefaultWeapon())
		{
			EquipBareHand();
		}
		return;
	}

	CurrentSkillPoints = FMath::Max(0, Progress.SkillPoints);
	TSet<FGameplayTag> Assigned;
	for (int32 Index = 0; Index < FMath::Min(QuickSlotWeaponTags.Num(), Progress.QuickSlots.Num()); ++Index)
	{
		const FGameplayTag& Tag = Progress.QuickSlots[Index];
		if (IsWeaponOwned(Tag) && !Assigned.Contains(Tag))
		{
			QuickSlotWeaponTags[Index] = Tag;
			Assigned.Add(Tag);
		}
		else if (Tag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Weapon restore: unavailable or duplicate weapon %s in slot %d."),
				*Tag.ToString(), Index);
		}
	}

	for (const auto& Style : Progress.SkillStyles)
	{
		FMVSkillTreeState* State = SkillTreeStates.Find(Style.Key);
		const TObjectPtr<UMVWeaponSkillMap>* Map = SkillBlueprint.Find(Style.Key);
		if (!State || !Map || !*Map)
		{
			continue;
		}

		for (const FMVSavedWeaponRow& Key : Style.Value.UnlockedRows)
		{
			const FDataTableRowHandle Row = MVWeaponProgressResolveRow(Key);
			const EMVSkillSlot Slot = Row.DataTable ? (*Map)->FindSkillSlot(Row) : EMVSkillSlot::NONE;
			const auto Candidates = (*Map)->GetSortedSkillCandidates(Slot);
			TArray<FName> ChainRows;
			if (Candidates.ContainsByPredicate([&](const auto& Candidate) { return Candidate.SkillRowHandle.RowHandle == Row; })
				&& (Slot == EMVSkillSlot::PassiveSkill ? IsValidPassiveCandidateRow(Row) : MVSkillExecutionData::ValidateDefinition(Row, ChainRows)))
			{
				State->UnlockedSkills.Add(FMVHashDataTableRowHandle(Row));
			}
		}

		for (const auto& Equipped : Style.Value.EquippedRows)
		{
			if (!MVSkillSelectionData::IsActiveSlot(Equipped.Key))
			{
				continue;
			}
			const FDataTableRowHandle Row = MVWeaponProgressResolveRow(Equipped.Value);
			const auto Candidates = (*Map)->GetSortedSkillCandidates(Equipped.Key);
			TArray<FName> ChainRows;
			if (Row.DataTable && Candidates.ContainsByPredicate([&](const auto& Candidate) { return Candidate.SkillRowHandle.RowHandle == Row; })
				&& MVSkillExecutionData::ValidateDefinition(Row, ChainRows)
				&& State->UnlockedSkills.Contains(FMVHashDataTableRowHandle(Row)))
			{
				State->EquippedSkill.Add(Equipped.Key, Row);
			}
		}
	}

	for (const auto& Passive : Progress.SelectedPassives)
	{
		const FDataTableRowHandle Row = MVWeaponProgressResolveRow(Passive.Value);
		if (!Row.DataTable)
		{
			continue;
		}
		const auto Candidates = GetPassiveCandidates(Passive.Key);
		if (Candidates.ContainsByPredicate([&Row](const FMVPassiveSkillCandidate& Candidate)
		{
			return Candidate.bUnlocked && Candidate.SkillRowHandle == Row;
		}))
		{
			SelectedPassiveByWeaponType.Add(Passive.Key, Row);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Weapon progress: saved passive %s is unavailable for %s."),
				*Row.RowName.ToString(), *Passive.Key.ToString());
		}
	}

	if (!QuickSlotWeaponTags.IsValidIndex(Progress.ActiveSlot)
		|| QuickSlotWeaponTags[Progress.ActiveSlot] != Progress.EquippedWeapon
		|| !TryEquipWeaponTag(Progress.EquippedWeapon, Progress.ActiveSlot, true))
	{
		if (QuickSlotWeaponTags.IsValidIndex(Progress.ActiveSlot) && !QuickSlotWeaponTags[Progress.ActiveSlot].IsValid())
		{
			ActiveQuickSlotIndex = Progress.ActiveSlot;
		}
		EquipBareHand(true);
	}
}

void UMVWeaponComponent::NotifyWeaponProgressRestored()
{
	OnOwnedWeaponsChanged.Broadcast();
	OnQuickSlotsChanged.Broadcast();
	OnPassiveOptionsChanged.Broadcast();
	if (Inventory)
	{
		Inventory->OnInventoryChanged.Broadcast(FGameplayTag());
	}
}

EMVSkillSelectionSaveStatus UMVWeaponComponent::StoreWeaponProgress(const bool bSaveImmediately) const
{
	UMVWorldStateSubsystem* State = WeaponSaveState.Get();
	if (bRestoringWeaponProgress) return EMVSkillSelectionSaveStatus::SuppressedDuringRestore;
	if (!State) return EMVSkillSelectionSaveStatus::Unavailable;

	FMVWeaponProgressSaveData Progress;
	Progress.bHasRecord = true;
	Progress.QuickSlots = QuickSlotWeaponTags;
	Progress.EquippedWeapon = CurrentWeaponState.ItemTag;
	Progress.ActiveSlot = ActiveQuickSlotIndex;
	Progress.SkillPoints = CurrentSkillPoints;

	for (const auto& Style : SkillTreeStates)
	{
		FMVSavedWeaponSkillStyle& SavedStyle = Progress.SkillStyles.Add(Style.Key);
		for (const FMVHashDataTableRowHandle& Unlocked : Style.Value.UnlockedSkills)
		{
			SavedStyle.UnlockedRows.Add(MVWeaponProgressMakeRowKey(Unlocked.RowHandle));
		}
		for (const auto& Equipped : Style.Value.EquippedSkill)
		{
			SavedStyle.EquippedRows.Add(Equipped.Key, MVWeaponProgressMakeRowKey(Equipped.Value));
		}
	}
	for (const auto& Passive : SelectedPassiveByWeaponType)
	{
		Progress.SelectedPassives.Add(Passive.Key, MVWeaponProgressMakeRowKey(Passive.Value));
	}

	FMVInventorySaveData SavedItems;
	if (Inventory)
	{
		SavedItems.bHasRecord = true;
		SavedItems.ItemCounts = Inventory->GetItems();
	}
	State->SetPlayerItemProgress(MoveTemp(SavedItems), MoveTemp(Progress));
	if (bSaveImmediately && !bDeferSkillSelectionSave)
	{
		return State->SaveActiveSlot() ? EMVSkillSelectionSaveStatus::Saved : EMVSkillSelectionSaveStatus::Failed;
	}
	return EMVSkillSelectionSaveStatus::NotRequested;
}
