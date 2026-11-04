// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/MVEquipmentEnums.h"
#include "Enum/MVSkillSlot.h"
#include "Struct/MVHashDataTableRowHandle.h" 

#include "MVSKillTree.generated.h"

/**
 * Struct to represent the state of a skill tree for a character. Unlocked Skill, Equipped Skill.
 * This struct contains information about the character's equipped style, unlocked skills, and skill levels.
 * One struct per One EquippedStyle.
 */

USTRUCT(BlueprintType)
struct FMVSkillTreeState
{
	GENERATED_BODY()

	UPROPERTY()
	EMVEquippedStyle EquippedStyle = EMVEquippedStyle::BareHand;

	UPROPERTY()
	TSet<FMVHashDataTableRowHandle> UnlockedSkills;

	UPROPERTY()
	TMap<EMVSkillSlot, FDataTableRowHandle> EquippedSkill;
	
};