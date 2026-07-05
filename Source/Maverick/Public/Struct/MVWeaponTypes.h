#pragma once

#include "CoreMinimal.h"
#include "Enum/MVEquipmentEnums.h"
#include "GameplayTagContainer.h"
#include "MVWeaponTypes.generated.h"

class USkeletalMesh;

/**
 * WeaponComponent가 소유하는 현재 장착 무기 상태.
 *
 * DataTable row 포인터를 런타임에 계속 들고 있지 않고, 장착 시점에 필요한 전투 값을 복사해
 * 컴포넌트 state로 보관한다. HitResolver는 타격이 확정되는 순간 이 상태에서 별도 스냅샷을 캡처한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVEquippedWeaponState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon", meta = (Categories = "Item.Weapon"))
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon")
	EMVEquippedStyle EquippedStyle = EMVEquippedStyle::BareHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon", meta = (ClampMin = "0.0"))
	float AttackPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon")
	EMVWeaponRangeType RangeType = EMVWeaponRangeType::Melee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Visual")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Visual")
	FName AttachSocketName = TEXT("hand_r_socket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon|Visual")
	FTransform AttachTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Weapon")
	bool bValid = false;
};

/**
 * 타격 확정 시점에 캡처해 FMVResolvedHitData로 전달하는 무기 값.
 *
 * 피격 처리, 히트 리액션, 로그 같은 후속 소비자는 WeaponComponent의 이후 장착 변경과 무관하게
 * 이 히트에 사용된 무기 값을 안정적으로 참조한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVWeaponHitSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Weapon|Hit", meta = (Categories = "Item.Weapon"))
	FGameplayTag ItemTag;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Weapon|Hit")
	EMVEquippedStyle EquippedStyle = EMVEquippedStyle::BareHand;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Weapon|Hit")
	float AttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Weapon|Hit")
	EMVWeaponRangeType RangeType = EMVWeaponRangeType::Melee;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Weapon|Hit")
	bool bValid = false;
};
