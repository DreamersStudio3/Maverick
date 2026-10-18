// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/MVWeaponSkillMap.h"

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
