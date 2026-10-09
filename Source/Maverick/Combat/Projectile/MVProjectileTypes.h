#pragma once

#include "CoreMinimal.h"
#include "Struct/MVHitTypes.h"

#include "MVProjectileTypes.generated.h"

class AActor;
class AMVCharacterBase;

UENUM(BlueprintType)
enum class EMVProjectileTeam : uint8
{
    Unknown,
    Player,
    Enemy
};

UENUM(BlueprintType)
enum class EMVProjectileTargetSelectionMode : uint8
{
    ProvidedOnly,
    Nearest,
    NearestInFront
};

/** Ability가 발사할 때 사용할 대상 선택 규칙. 발사체가 비행 중 다시 조회하지 않는다. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVProjectileTargetingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Projectile|Targeting")
    EMVProjectileTargetSelectionMode Mode = EMVProjectileTargetSelectionMode::ProvidedOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Projectile|Targeting", meta = (ClampMin = "0.0"))
    float SearchRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Projectile|Targeting", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float FrontHalfAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Projectile|Targeting")
    bool bPreferCurrentTarget = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Projectile|Targeting")
    bool bRequireClearPath = true;
};

/**
 * Ability가 발사 순간의 공격 정보를 복사해 발사체에 전달한다.
 *
 * 발사체가 이동하는 동안 현재 장착 무기나 현재 Ability에서 이 값을 다시 읽지 않는다.
 * 피격 대상과 실제 충돌 위치는 발사 시점에 알 수 없으므로 여기에 저장하지 않는다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVProjectileLaunchContext
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TWeakObjectPtr<AMVCharacterBase> Attacker;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    EMVProjectileTeam Team = EMVProjectileTeam::Unknown;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    int32 AttackInstanceId = INDEX_NONE;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    EMVAttackTypes AttackTypes = EMVAttackTypes::NormalAttack;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    FVector LaunchDirection = FVector::ZeroVector;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> HomingTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile|Weapon")
    FMVWeaponHitSnapshot WeaponSnapshot;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile|Damage")
    float DamageMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile|Damage")
    float GroggyDamageMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile|Damage")
    float PoiseDamage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile|Reaction")
    EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile|Launch")
    FMVHitLaunchData HitLaunchData;
};