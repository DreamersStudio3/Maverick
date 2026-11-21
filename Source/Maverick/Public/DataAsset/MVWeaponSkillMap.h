// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/MVCombatActionTypes.h"
#include "Enum/MVSkillSlot.h"
#include "Enum/MVEquipmentEnums.h"
#include "Struct/MVHashDataTableRowHandle.h" 
#include "GameplayTagContainer.h"

#include "MVWeaponSkillMap.generated.h"

/** 무기 스타일에 고정된 4종 Basic Attack 시작 Row 모음. */
USTRUCT(BlueprintType)
struct FMVWeaponBasicAttackMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Attack")
	FDataTableRowHandle LightAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Attack")
	FDataTableRowHandle DodgeLightAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Attack")
	FDataTableRowHandle SprintLightAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Attack")
	FDataTableRowHandle ChargeAttack;

	const FDataTableRowHandle* Find(EMVCombatActionTypes ActionType) const;
};

/** 스킬 후보 Row와 해금 비용을 함께 보관하는 데이터 에셋 항목. */

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

	// PassiveSkill entries only. An empty container is not a selectable passive candidate.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passive", meta = (Categories = "Weapon.Type"))
	FGameplayTagContainer PassiveWeaponTypes;

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

/**
 * 한 무기 스타일에서 사용할 모든 전투 동작의 정적 정의.
 *
 * Basic Attack은 해금·장착 과정 없이 BasicAttacks의 고정 Row를 사용한다.
 * 스킬 집합은 WeaponComponent가 스타일별 해금·장착 상태를 구성하는 원본이며,
 * 런타임 진행 상태 자체는 이 데이터 에셋에 저장하지 않는다.
 */
UCLASS()
class MAVERICK_API UMVWeaponSkillMap : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EMVEquippedStyle WeaponStyle = EMVEquippedStyle::BareHand;

	UPROPERTY(EditAnywhere, Category = "Basic Attack")
	FMVWeaponBasicAttackMap BasicAttacks;

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

	/** QWER의 무료 Ready 기본 행. 미설정 시 유효 후보를 정렬해 결정한다. */
	UPROPERTY(EditAnywhere, Category = "Skills|Defaults")
	TMap<EMVSkillSlot, FDataTableRowHandle> DefaultSkillRows;

	const TSet<FMVWeaponSkillMapData>* GetSkillCandidates(EMVSkillSlot Slot) const;
	TArray<FMVWeaponSkillMapData> GetSortedSkillCandidates(EMVSkillSlot Slot) const;
	bool GetDefaultSkillRow(EMVSkillSlot Slot, FDataTableRowHandle& OutRow) const;
	/** 후보 형식·중복 슬롯·기본 행 오류 수집. 게임 상태와 자산을 수정하지 않는다. */
	bool ValidateSkillCandidates(TArray<FString>& OutErrors) const;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	
public:
	bool GetBasicAttackRow(EMVCombatActionTypes ActionType, FDataTableRowHandle& OutRowHandle) const;

	// Returns the skill slot for the given row handle, or EMVSkillSlot::None if not found
	EMVSkillSlot FindSkillSlot(const FDataTableRowHandle& InRowHandle) const;
	
	// Returns the required points for the given row handle, or -1 if not found
	int32 GetRequiredPointsForSkill(const FDataTableRowHandle& InRowHandle) const;
};
