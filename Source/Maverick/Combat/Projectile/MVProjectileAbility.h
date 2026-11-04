#pragma once

#include "CoreMinimal.h"
#include "Combat/MVAbilityBase.h"
#include "Combat/Projectile/MVProjectileTypes.h"

#include "MVProjectileAbility.generated.h"

class AActor;
class AMVProjectileBase;

/** 발사 위치와 방향을 Ability가 결정한 결과. 유효하지 않으면 발사를 진행하지 않는다. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVProjectileSpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    bool bValid = false;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    FVector Direction = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Maverick|Projectile")
    TObjectPtr<AActor> HomingTarget = nullptr;
};

/**
 * 기존 Ability 실행에서 발사 시점의 위치와 공격 정보를 준비하는 원거리 Ability.
 *
 * 기본 위치는 캐릭터 메시 소켓, 방향은 캐릭터 정면이다.
 * ResolveSpawnInfo를 Blueprint에서 재정의하면 무기 소켓이나 조준 방향을 사용할 수 있다.
 * 발사 정보를 검증하고 유도 대상이 필요하면 발사 순간에 한 번 선택한다.
 * 부모 StartAbility가 비용을 소모하면 발사체를 만든다.
 * Combat에 공격 번호와 발사체를 등록한 뒤 충돌을 켠다.
 * 늦은 적중의 후속 효과는 Combat의 실행 기록 처리에 맡긴다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVProjectileAbility : public UMVAbilityBase
{
    GENERATED_BODY()

public:
    virtual void StartAbility_Implementation(int32 AbilityIndex) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Projectile")
    FMVProjectileSpawnInfo ResolveSpawnInfo(int32 AbilityIndex);
    virtual FMVProjectileSpawnInfo ResolveSpawnInfo_Implementation(int32 AbilityIndex);

    bool TryBuildLaunchContext(
        int32 AbilityIndex,
        FMVProjectileLaunchContext& OutContext,
        FTransform& OutSpawnTransform);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile")
    TSubclassOf<AMVProjectileBase> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile|Targeting")
    FMVProjectileTargetingSettings TargetingSettings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile|Spawn")
    FName LaunchSocket = NAME_None;

    // 캐릭터 기준 앞·오른쪽·위쪽 거리(cm).
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile|Spawn")
    FVector LaunchOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile|Hit")
    EMVAttackTypes ProjectileAttackType = EMVAttackTypes::NormalAttack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile|Hit")
    EMVActionHitReactionType ProjectileHitReaction = EMVActionHitReactionType::Flinch;
};