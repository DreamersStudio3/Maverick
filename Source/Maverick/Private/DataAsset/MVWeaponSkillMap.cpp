// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/MVWeaponSkillMap.h"
#include "Combat/MVAbilityBase.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Weapon/Skill/MVSkillSelectionData.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#if WITH_EDITOR
EDataValidationResult UMVWeaponSkillMap::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult Parent = Super::IsDataValid(Context);
	TArray<FString> Errors;
	ValidateSkillCandidates(Errors);
	for (const FString& Error : Errors) Context.AddError(FText::FromString(Error));
	return !Errors.IsEmpty() || Parent == EDataValidationResult::Invalid
		? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

namespace
{
bool MVSkillMapCandidateRowValid(const FMVWeaponSkillMapData& Candidate, const EMVSkillSlot Slot)
{
	const auto& Handle = Candidate.SkillRowHandle.RowHandle;
	if (!Handle.DataTable || Handle.RowName.IsNone() || !Handle.DataTable->GetRowStruct()
		|| !Handle.DataTable->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct())) return false;
	const auto* Row = Handle.DataTable->FindRow<FMVSkillDataTableColumn>(Handle.RowName, TEXT("SkillSelection.Validate"), false);
	if (!Row) return false;
	if (Row->ImplementationState == EMVSkillImplementationState::Prototype)
		return MVSkillSelectionData::IsActiveSlot(Slot);
	return Row->ImplementationState == EMVSkillImplementationState::Ready && Row->AbilityReference != nullptr;
}
FString MVSkillMapRowSortKey(const FDataTableRowHandle& Row)
{
	return GetPathNameSafe(Row.DataTable) + TEXT(":") + Row.RowName.ToString();
}
}

const TSet<FMVWeaponSkillMapData>* UMVWeaponSkillMap::GetSkillCandidates(const EMVSkillSlot Slot) const
{
	switch (Slot)
	{
	case EMVSkillSlot::PassiveSkill: return &PassiveSkill;
	case EMVSkillSlot::QSkill: return &QSkill;
	case EMVSkillSlot::WSkill: return &WSkill;
	case EMVSkillSlot::ESkill: return &ESkill;
	case EMVSkillSlot::RSkill: return &RSkill;
	default: return nullptr;
	}
}

TArray<FMVWeaponSkillMapData> UMVWeaponSkillMap::GetSortedSkillCandidates(const EMVSkillSlot Slot) const
{
	TArray<FMVWeaponSkillMapData> Result;
	const auto* Candidates = GetSkillCandidates(Slot);
	if (!Candidates) return Result;
	for (const auto& Candidate : *Candidates)
	{
		if (!MVSkillMapCandidateRowValid(Candidate, Slot)) continue;
		int32 MembershipCount = 0;
		for (const auto& Definition : MVSkillSelectionData::GetSlots())
		{
			if (GetSkillCandidates(Definition.Slot)->Contains(Candidate)) ++MembershipCount;
		}
		if (MembershipCount == 1) Result.Add(Candidate);
	}
	Result.Sort([](const auto& A, const auto& B)
	{
		return MVSkillMapRowSortKey(A.SkillRowHandle.RowHandle) < MVSkillMapRowSortKey(B.SkillRowHandle.RowHandle);
	});
	return Result;
}

bool UMVWeaponSkillMap::GetDefaultSkillRow(const EMVSkillSlot Slot, FDataTableRowHandle& OutRow) const
{
	OutRow = {};
	if (!MVSkillSelectionData::IsActiveSlot(Slot)) return false;
	const auto Candidates = GetSortedSkillCandidates(Slot);
	auto CanDefault = [](const FMVWeaponSkillMapData& Entry)
	{
		const auto* Row = Entry.SkillRowHandle.RowHandle.GetRow<FMVSkillDataTableColumn>(TEXT("SkillSelection.Default"));
		return Entry.RequiredPoints <= 0 && Row && Row->ImplementationState == EMVSkillImplementationState::Ready;
	};
	if (const auto* Explicit = DefaultSkillRows.Find(Slot))
	{
		for (const auto& Candidate : Candidates)
			if (Candidate.SkillRowHandle.RowHandle == *Explicit && CanDefault(Candidate)) { OutRow = *Explicit; return true; }
	}
	for (const auto& Candidate : Candidates)
		if (CanDefault(Candidate)) { OutRow = Candidate.SkillRowHandle.RowHandle; return true; }
	return false;
}

bool UMVWeaponSkillMap::ValidateSkillCandidates(TArray<FString>& OutErrors) const
{
	OutErrors.Reset();
	for (const auto& Definition : MVSkillSelectionData::GetSlots())
	{
		for (const auto& Candidate : *GetSkillCandidates(Definition.Slot))
		{
			if (!MVSkillMapCandidateRowValid(Candidate, Definition.Slot))
				OutErrors.Add(FString::Printf(TEXT("Invalid candidate Slot=%d Row=%s"), int32(Definition.Slot), *MVSkillMapRowSortKey(Candidate.SkillRowHandle.RowHandle)));
			int32 Count = 0;
			for (const auto& Other : MVSkillSelectionData::GetSlots()) if (GetSkillCandidates(Other.Slot)->Contains(Candidate)) ++Count;
			if (Count > 1) OutErrors.Add(TEXT("Candidate registered in multiple slots"));
		}
	}
	for (const auto& Default : DefaultSkillRows)
	{
		FDataTableRowHandle Resolved;
		if (!GetDefaultSkillRow(Default.Key, Resolved) || Resolved != Default.Value)
			OutErrors.Add(FString::Printf(TEXT("Invalid default Slot=%d"), int32(Default.Key)));
	}
	return OutErrors.IsEmpty();
}

const FDataTableRowHandle* FMVWeaponBasicAttackMap::Find(const EMVCombatActionTypes ActionType) const
{
	switch (ActionType)
	{
	case EMVCombatActionTypes::LightAttack:
		return &LightAttack;
	case EMVCombatActionTypes::DodgeLightAttack:
		return &DodgeLightAttack;
	case EMVCombatActionTypes::SprintLightAttack:
		return &SprintLightAttack;
	case EMVCombatActionTypes::ChargeAttack:
		return &ChargeAttack;
	default:
		return nullptr;
	}
}

bool UMVWeaponSkillMap::GetBasicAttackRow(
	const EMVCombatActionTypes ActionType,
	FDataTableRowHandle& OutRowHandle) const
{
	const FDataTableRowHandle* RowHandle = BasicAttacks.Find(ActionType);
	if (!RowHandle || !RowHandle->DataTable || RowHandle->RowName.IsNone())
	{
		OutRowHandle = FDataTableRowHandle();
		return false;
	}

	OutRowHandle = *RowHandle;
	return true;
}

EMVSkillSlot UMVWeaponSkillMap::FindSkillSlot(const FDataTableRowHandle& InRowHandle) const
{
	FMVWeaponSkillMapData SearchKey{ InRowHandle };

	if (PassiveSkill.Contains(SearchKey))
	{
		return EMVSkillSlot::PassiveSkill;
	}
	else if (QSkill.Contains(SearchKey))
	{
		return EMVSkillSlot::QSkill;
	}
	else if (WSkill.Contains(SearchKey))
	{
		return EMVSkillSlot::WSkill;
	}
	else if (ESkill.Contains(SearchKey))
	{
		return EMVSkillSlot::ESkill;
	}
	else if (RSkill.Contains(SearchKey))
	{
		return EMVSkillSlot::RSkill;
	}

	return EMVSkillSlot::NONE;
}

int32 UMVWeaponSkillMap::GetRequiredPointsForSkill(const FDataTableRowHandle& InRowHandle) const
{
	const FMVWeaponSkillMapData SearchKey{ InRowHandle };

	const TSet<FMVWeaponSkillMapData>* SkillSets[] =
	{
		&PassiveSkill,
		&QSkill,
		&WSkill,
		&ESkill,
		&RSkill
	};

	for (const TSet<FMVWeaponSkillMapData>* SkillSet : SkillSets)
	{
		if (const FMVWeaponSkillMapData* Found = SkillSet->Find(SearchKey))
		{
			return Found->RequiredPoints;
		}
	}

	return -1;

}
