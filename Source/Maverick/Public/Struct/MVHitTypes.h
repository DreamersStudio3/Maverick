#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Struct/MVWeaponTypes.h"
#include "Tables/MVActionTableTypes.h"
#include "MVHitTypes.generated.h"

class AMVCharacterBase;

/**
 * 공격 Ability마다 정하는 피격 밀림 값.
 *
 * HitReaction row에서 Launch 사용을 켜두면 이 값으로 실제 밀림이 걸린다.
 * Distance는 초당 거리가 아니라 총 목표거리다. Distance를 Duration으로 나누면 가로 속도가 나오고,
 * VerticalSpeed를 넣으면 위로 뜨는 힘도 같이 들어간다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitLaunchData
{
	GENERATED_BODY()

	// 가로로 밀어낼 총 목표거리. Distance 500, Duration 3이면 500/3이라 초당 약 166.7cm씩 밀린다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Launch", meta = (ClampMin = "0.0", Units = "cm", ForceUnits = "cm"))
	float LaunchDistance = 0.0f;

	// 가로 밀림을 유지할 시간. 0으로 두면 Distance를 넣어도 가로 Launch는 거의 걸리지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Launch", meta = (ClampMin = "0.0", Units = "s"))
	float LaunchDuration = 0.0f;

	// 위로 띄우는 속도. cm/s 값 그대로 LaunchCharacter에 들어가고, Duration이 끝나면 상승 속도를 끊어 낙하로 넘긴다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Launch", meta = (Units = "cm/s", ForceUnits = "cm/s"))
	float LaunchVerticalSpeed = 0.0f;
};

/**
 * 충돌 필터링 이후 HitResolver에 넘기는 원본 타격 요청.
 *
 * 공격자, 피격자, 대미지 계수, 피격 타입을 채운 뒤 HitResult의 ImpactPoint/ImpactNormal을 같이 넘긴다.
 * 방향은 HitResolver가 공격자 위치에서 피격자 위치로 계산한다. Ability마다 HitDirection을 따로 만들지 않아도 된다.
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
	int32 AttackInstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Damage", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Damage", meta = (ClampMin = "0.0"))
	float GroggyDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Reaction")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Launch")
	FMVHitLaunchData HitLaunchData;

	// HitResult.ImpactPoint에 해당하는 실제 충돌 지점. 방향 계산에는 끼지 않고, VFX나 디버그에서 어디를 맞았는지 보려고 남겨둔다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context", meta = (Units = "cm", ForceUnits = "cm"))
	FVector HitLocation = FVector::ZeroVector;

	// HitResult.ImpactNormal에 해당하는 표면 노멀. Launch 방향을 흔들지는 않고, 충돌 표면 문맥이 필요할 때 참고한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector ImpactNormal = FVector::ZeroVector;
};

/**
 * 공격력 공식이 아닌 외부 로직에서 최종 계산한 피해를
 * 기존 피격 처리 경로로 전달하기 위한 요청 데이터.
 *
 * 대표 사용처:
 * - 대상 최대 체력 비례 피해
 * - 지속 피해
 * - 고정 피해
 * - 상태 효과에 의한 추가 피해
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVDirectDamageRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AMVCharacterBase> Attacker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AMVCharacterBase> Victim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	int32 AttackInstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float FinalDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float GroggyDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FMVHitLaunchData HitLaunchData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector ImpactNormal = FVector::ZeroVector;
};

/**
 * HitResolver가 계산해 피격자에게 넘기는 최종 타격 결과.
 *
 * 무기 스냅샷, 스탯, 대미지 계수, 충돌 정보를 조합해 채운다.
 * HitReactionComponent는 여기 들어있는 HitDirection을 그대로 Launch와 방향 선택에 사용한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVResolvedHitData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Attacker = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit")
	TObjectPtr<AMVCharacterBase> Victim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit")
	int32 AttackInstanceId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit", meta = (Categories = "Character"))
	FGameplayTag AttackerCharacterIndexCode;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit", meta = (Categories = "Character"))
	FGameplayTag VictimCharacterIndexCode;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Damage")
	float CharacterAttackPower = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Weapon")
	FMVWeaponHitSnapshot WeaponSnapshot;

	// 기존 BP/디버그 경로 호환용으로 남겨둔 스냅샷 공격력의 평탄화 값.
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

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Launch")
	FMVHitLaunchData HitLaunchData;

	// 요청에서 넘어온 충돌 지점. 후속 연출이나 로그에서 다시 확인할 수 있게 남겨둔다.
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Context", meta = (Units = "cm", ForceUnits = "cm"))
	FVector HitLocation = FVector::ZeroVector;

	// 요청에서 넘어온 표면 노멀. 디버그와 후속 연출 문맥에서 다시 참고할 수 있게 남겨둔다.
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Context")
	FVector ImpactNormal = FVector::ZeroVector;

	// HitResolver가 확정한 월드 밀림 방향. 피격자 위치에서 공격자 위치를 뺀 값이라 Actor forward와 상관없이 같은 방향을 쓴다.
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Hit|Context")
	FVector HitDirection = FVector::ZeroVector;
};

/**
 * 충돌 컴포넌트가 HitResolver에 덧붙일 수 있는 충돌 문맥.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitResolveContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context", meta = (Units = "cm", ForceUnits = "cm"))
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Hit|Context")
	FVector ImpactNormal = FVector::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnHitResolvedSignature, const FMVResolvedHitData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDamagedSignature, const FMVResolvedHitData&, HitData);
