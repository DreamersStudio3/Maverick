#include "Weapon/Skill/MVSkillSelectionData.h"
#include "DataAsset/MVWeaponSkillMap.h"
#include "Public/Struct/MVSkillTree.h"
#include "Public/Tags/MVGameplayTags.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Weapon/Skill/MVSkillExecutionData.h"

const TArray<FMVSkillSlotDefinition>& MVSkillSelectionData::GetSlots()
{
	static const TArray<FMVSkillSlotDefinition> Slots = []
	{
		TArray<FMVSkillSlotDefinition> Result;
		const EMVSkillSlot Order[] = {EMVSkillSlot::PassiveSkill, EMVSkillSlot::QSkill, EMVSkillSlot::WSkill, EMVSkillSlot::ESkill, EMVSkillSlot::RSkill};
		const FText Names[] = {NSLOCTEXT("SkillSelection", "Passive", "패시브"), FText::FromString(TEXT("Q")), FText::FromString(TEXT("W")), FText::FromString(TEXT("E")), FText::FromString(TEXT("R"))};
		const FGameplayTag Inputs[] = {FGameplayTag(), MVGameplayTags::Action_Input_Skill_Q, MVGameplayTags::Action_Input_Skill_W, MVGameplayTags::Action_Input_Skill_E, MVGameplayTags::Action_Input_Skill_R};
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(Order); ++Index)
		{
			FMVSkillSlotDefinition& Entry = Result.AddDefaulted_GetRef();
			Entry.Slot = Order[Index];
			Entry.DisplayName = Names[Index];
			Entry.InputTag = Inputs[Index];
			Entry.FallbackIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture")));
		}
		return Result;
	}();
	return Slots;
}

bool MVSkillSelectionData::IsActiveSlot(const EMVSkillSlot Slot)
{
	return Slot == EMVSkillSlot::QSkill || Slot == EMVSkillSlot::WSkill || Slot == EMVSkillSlot::ESkill || Slot == EMVSkillSlot::RSkill;
}

void MVSkillSelectionData::InitializeActiveSlot(const UMVWeaponSkillMap& Map, const EMVSkillSlot Slot, FMVSkillTreeState& State)
{
	if (!IsActiveSlot(Slot)) return;
	const auto Candidates = Map.GetSortedSkillCandidates(Slot);
	for (const FMVWeaponSkillMapData& Candidate : Candidates)
	{
		TArray<FName> Rows;
		if (Candidate.RequiredPoints <= 0 && MVSkillExecutionData::ValidateDefinition(Candidate.SkillRowHandle.RowHandle, Rows))
			State.UnlockedSkills.Add(Candidate.SkillRowHandle);
	}
	// 기존 선택 보존. 저장된 행의 유효성 검사는 복원 경로의 책임.
	if (State.EquippedSkill.Contains(Slot)) return;
	FDataTableRowHandle Default;
	TArray<FName> Rows;
	if (Map.GetDefaultSkillRow(Slot, Default) && MVSkillExecutionData::ValidateDefinition(Default, Rows, false))
	{
		State.EquippedSkill.Add(Slot, Default);
		return;
	}
	for (const auto& Candidate : Candidates)
	{
		if (Candidate.RequiredPoints <= 0 && MVSkillExecutionData::ValidateDefinition(Candidate.SkillRowHandle.RowHandle, Rows, false))
		{
			State.EquippedSkill.Add(Slot, Candidate.SkillRowHandle.RowHandle);
			return;
		}
	}
}
