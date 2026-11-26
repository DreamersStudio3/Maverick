#pragma once

#include "Weapon/Skill/MVSkillSelectionTypes.h"

class UMVWeaponSkillMap;
struct FMVSkillTreeState;

/** 선택 자료의 공통 처리. 상태를 소유하지 않고 전달된 한 슬롯만 초기화한다. */
namespace MVSkillSelectionData
{
	MAVERICK_API const TArray<FMVSkillSlotDefinition>& GetSlots();
	MAVERICK_API bool IsActiveSlot(EMVSkillSlot Slot);
	MAVERICK_API void InitializeActiveSlot(const UMVWeaponSkillMap& Map, EMVSkillSlot Slot, FMVSkillTreeState& State);
}
