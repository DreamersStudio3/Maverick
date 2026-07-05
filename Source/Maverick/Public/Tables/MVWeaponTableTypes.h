#pragma once

#include "CoreMinimal.h"
#include "Enum/MVEquipmentEnums.h"
#include "Tables/MVItemTableTypes.h"
#include "MVWeaponTableTypes.generated.h"

class USkeletalMesh;

/**
 * 무기 아이템의 장착/전투 선택 데이터.
 *
 * 개별 공격과 스킬의 AbilityReference, 대미지, 비용, 체인 정보는 기존
 * FMVSkillDataTableColumn row가 소유한다. 이 row는 현재 장착 무기의 공통 공격력과
 * 장비 스타일, 근거리/원거리 실행 유형, 장착 메시 정보를 제공한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVWeaponTableRow : public FMVItemTableRow
{
	GENERATED_BODY()

	FMVWeaponTableRow()
	{
		ItemType = EMVItemType::Equipment;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Weapon")
	EMVEquippedStyle EquippedStyle = EMVEquippedStyle::BareHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Weapon", meta = (ClampMin = "0.0"))
	float AttackPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Weapon")
	EMVWeaponRangeType RangeType = EMVWeaponRangeType::Melee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Weapon|Visual")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Weapon|Visual")
	FName AttachSocketName = TEXT("hand_r_socket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Item|Weapon|Visual")
	FTransform AttachTransform = FTransform::Identity;
};
