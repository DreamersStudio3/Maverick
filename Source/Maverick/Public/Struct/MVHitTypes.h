#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Struct/MVWeaponTypes.h"
#include "Tables/MVActionTableTypes.h"
#include "MVHitTypes.generated.h"

class AMVCharacterBase;

/**
 * 충돌 필터링 이후 HitResolver에 전달되는 원본 타격 요청.
 *
 * 충돌 컴포넌트는 이미 자기 자신 제외, 액션 1회당 중복 타격 제한 같은 후보 필터링을 끝낸 뒤
 * 공격자/피격자와 AbilityData에서 확정된 피해 배율을 넘긴다. HitResolver는 공격자의
 * WeaponComponent에서 타격 순간의 무기 스냅샷을 캡처한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitResolveRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Attacker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Victim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Damage", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Damage", meta = (ClampMin = "0.0"))
	float GroggyDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Reaction")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector HitDirection = FVector::ZeroVector;
};

/**
 * 충돌 후보 필터링 이후 HitResolver가 계산해 피격자에게 전달하는 피해 결과.
 *
 * 충돌 컴포넌트는 공격자, 피격자와 확정된 공격 계수를 넘기고, 이 구조체는 HitResolver가 공격자/피격자
 * 스탯, 계수, 무기 스냅샷을 조합해 만든 HP 피해량, 피격 반응 유형, 후속 컴포넌트가 참고할 문맥을 담는다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVResolvedHitData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Attacker = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Victim = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit", meta = (Categories = "Character"))
	FGameplayTag AttackerCharacterIndexCode;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit", meta = (Categories = "Character"))
	FGameplayTag VictimCharacterIndexCode;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float CharacterAttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Weapon")
	FMVWeaponHitSnapshot WeaponSnapshot;

	// 기존 BP/디버그 경로 호환용으로 보관하는 스냅샷 공격력의 평탄화 값이다.
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float WeaponAttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float VictimDefence = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float DamageMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float GroggyDamageMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float FinalDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float GroggyDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Reaction")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Context")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Context")
	FVector HitDirection = FVector::ZeroVector;
};

/**
 * 충돌 컴포넌트가 HitResolver에 추가로 넘길 수 있는 선택 문맥.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitResolveContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector HitDirection = FVector::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnHitResolvedSignature, const FMVResolvedHitData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDamagedSignature, const FMVResolvedHitData&, HitData);
