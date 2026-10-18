// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/MVSkillSlot.h"
#include "Enum/MVEquipmentEnums.h"
#include "Struct/MVHashDataTableRowHandle.h" 

#include "MVWeaponSkillMap.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMVWeaponSkillMapData
{
	GENERATED_BODY()

	FMVWeaponSkillMapData()
		: SkillRowHandle(), RequiredPoints(0)
	{
	}
	FMVWeaponSkillMapData(const FDataTableRowHandle& InRowHandle)
		: SkillRowHandle(InRowHandle), RequiredPoints(0)
	{
	}

	FMVWeaponSkillMapData(const FDataTableRowHandle& InRowHandle, int32 InRequiredPoints)
		: SkillRowHandle(InRowHandle), RequiredPoints(InRequiredPoints)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	FMVHashDataTableRowHandle SkillRowHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredPoints = 0;

	bool operator==(const FMVWeaponSkillMapData& Other) const
	{
		return SkillRowHandle == Other.SkillRowHandle;
	}

};

// Hash function for FMVWeaponSkillMapData to be used in TSet
// Only the SkillRowHandle is used for hashing and equality comparison
FORCEINLINE uint32 GetTypeHash(const FMVWeaponSkillMapData& Input)
{
	return GetTypeHash(Input.SkillRowHandle);
}

UCLASS()
class MAVERICK_API UMVWeaponSkillMap : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EMVEquippedStyle WeaponStyle = EMVEquippedStyle::BareHand;

	UPROPERTY(EditAnywhere)
	TSet<FMVWeaponSkillMapData> PassiveSkill;
	UPROPERTY(EditAnywhere)
	TSet<FMVWeaponSkillMapData> QSkill;
	UPROPERTY(EditAnywhere)
	TSet<FMVWeaponSkillMapData> WSkill;
	UPROPERTY(EditAnywhere)
	TSet<FMVWeaponSkillMapData> ESkill;
	UPROPERTY(EditAnywhere)
	TSet<FMVWeaponSkillMapData> RSkill;
	
public:
	// Returns the skill slot for the given row handle, or EMVSkillSlot::None if not found
	EMVSkillSlot FindSkillSlot(const FDataTableRowHandle& InRowHandle) const;
	
	// Returns the required points for the given row handle, or -1 if not found
	int32 GetRequiredPointsForSkill(const FDataTableRowHandle& InRowHandle) const;
};
