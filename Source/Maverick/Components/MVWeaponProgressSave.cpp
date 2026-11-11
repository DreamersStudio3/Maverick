#include "Components/MVWeaponComponent.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "System/MVWorldStateSubsystem.h"

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
		RestoreWeaponProgress(FMVWeaponProgressSaveData());
	}
}

void UMVWeaponComponent::HandleWeaponSaveLoaded(FString SlotName)
{
	if (UMVWorldStateSubsystem* State = WeaponSaveState.Get())
	{
		const bool bHadRecord = State->GetWeaponProgress().bHasRecord;
		RestoreWeaponProgress(State->GetWeaponProgress());
		if (!bHadRecord)
		{
			StoreWeaponProgress();
		}
	}
}

void UMVWeaponComponent::RestoreWeaponProgress(const FMVWeaponProgressSaveData& Progress)
{
	TGuardValue<bool> RestoreGuard(bRestoringWeaponProgress, true);
	OwnedWeapons.Reset();
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
		if (!TryInitializeDefaultWeapon())
		{
			EquipBareHand();
		}
		return;
	}

	CurrentSkillPoints = FMath::Max(0, Progress.SkillPoints);
	for (const auto& Owned : Progress.OwnedWeapons)
	{
		const FDataTableRowHandle Handle = MVWeaponProgressResolveRow(Owned.Value);
		const FMVWeaponTableRow* Row = Handle.DataTable
			? ResolveWeaponRow(Handle, TEXT("RestoreWeaponProgress")) : nullptr;
		if (Owned.Key.IsValid() && Row && Row->ItemTag == Owned.Key && Row->ItemTag != BareHandWeapon.ItemTag)
		{
			OwnedWeapons.Add(Owned.Key, Handle);
		}
	}

	TSet<FGameplayTag> Assigned;
	for (int32 Index = 0; Index < FMath::Min(QuickSlotWeaponTags.Num(), Progress.QuickSlots.Num()); ++Index)
	{
		const FGameplayTag& Tag = Progress.QuickSlots[Index];
		if (OwnedWeapons.Contains(Tag) && !Assigned.Contains(Tag))
		{
			QuickSlotWeaponTags[Index] = Tag;
			Assigned.Add(Tag);
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
			if (Slot != EMVSkillSlot::NONE
				&& (Slot != EMVSkillSlot::PassiveSkill || IsValidPassiveCandidateRow(Row)))
			{
				State->UnlockedSkills.Add(FMVHashDataTableRowHandle(Row));
			}
		}

		for (const auto& Equipped : Style.Value.EquippedRows)
		{
			if (Equipped.Key == EMVSkillSlot::NONE || Equipped.Key == EMVSkillSlot::PassiveSkill)
			{
				continue;
			}
			const FDataTableRowHandle Row = MVWeaponProgressResolveRow(Equipped.Value);
			if (Row.DataTable && (*Map)->FindSkillSlot(Row) == Equipped.Key
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

	if (!TryEquipWeaponTag(Progress.EquippedWeapon, Progress.ActiveSlot, true))
	{
		if (QuickSlotWeaponTags.IsValidIndex(Progress.ActiveSlot) && !QuickSlotWeaponTags[Progress.ActiveSlot].IsValid())
		{
			ActiveQuickSlotIndex = Progress.ActiveSlot;
		}
		EquipBareHand(true);
	}
	OnOwnedWeaponsChanged.Broadcast();
	OnQuickSlotsChanged.Broadcast();
	OnPassiveOptionsChanged.Broadcast();
}

void UMVWeaponComponent::StoreWeaponProgress(const bool bSaveImmediately) const
{
	UMVWorldStateSubsystem* State = WeaponSaveState.Get();
	if (bRestoringWeaponProgress || !State)
	{
		return;
	}

	FMVWeaponProgressSaveData Progress;
	Progress.bHasRecord = true;
	Progress.QuickSlots = QuickSlotWeaponTags;
	Progress.EquippedWeapon = CurrentWeaponState.ItemTag;
	Progress.ActiveSlot = ActiveQuickSlotIndex;
	Progress.SkillPoints = CurrentSkillPoints;

	for (const auto& Owned : OwnedWeapons)
	{
		Progress.OwnedWeapons.Add(Owned.Key, MVWeaponProgressMakeRowKey(Owned.Value));
	}
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

	State->SetWeaponProgress(MoveTemp(Progress));
	if (bSaveImmediately)
	{
		State->SaveActiveSlot();
	}
}
