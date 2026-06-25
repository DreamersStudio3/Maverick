#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Tables/MVActionTableTypes.h"
#include "MVHitTypes.generated.h"

class AMVCharacterBase;

/**
 * 충돌 필터링 이후 HitResolver에 전달되는 원본 타격 요청.
 *
 * 충돌 컴포넌트는 이미 자기 자신 제외, 액션 1회당 중복 타격 제한 같은 후보 필터링을 끝낸 뒤
 * 공격자/피격자/공격 row 문맥을 넘긴다. 정식 흐름에서는 WeaponComponent가 항상 현재 무기
 * 스탯을 제공하며, 아이템 무기가 없을 때도 맨손 무기를 기본 장착한 것으로 취급한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitResolveRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Attacker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Victim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit")
	FName ActionRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Damage", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Reaction")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

	// WeaponComponent 계약 전까지 프로토타입 호출자가 주입하는 현재 무기 공격력이다.
	// 이후 HitResolver가 공격자의 WeaponComponent에서 맨손 포함 현재 무기 스탯을 직접 조회한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Weapon", meta = (ClampMin = "0.0"))
	float WeaponAttackPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector HitDirection = FVector::ZeroVector;
};

/**
 * 충돌 후보 필터링 이후 HitResolver가 계산해 피격자에게 전달하는 피해 결과.
 *
 * 충돌 컴포넌트는 공격자, 피격자, 공격 row 문맥을 넘기고, 이 구조체는 HitResolver가 공격자/피격자
 * 스탯과 확정된 공격 계수를 조합해 만든 HP 피해량, 피격 반응 유형, 후속 컴포넌트가 참고할 문맥을 담는다.
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

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit")
	FName ActionRowName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float CharacterAttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float WeaponAttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float VictimDefence = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float DamageMultiplier = 1.0f;

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
