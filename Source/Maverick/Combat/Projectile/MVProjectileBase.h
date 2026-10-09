#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/Projectile/MVProjectileTypes.h"

#include "MVProjectileBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class AMVCharacterBase;
class AMVProjectileBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FMVOnProjectileHitSignature,
    AMVProjectileBase*, Projectile,
    const FMVResolvedHitData&, HitData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FMVOnProjectileFinishedSignature,
    AMVProjectileBase*, Projectile,
    int32, AttackInstanceId);

/**
 * 원거리 공격의 이동, 충돌 후보 검사, 적중 요청과 종료를 담당하는 발사체 기반 Actor.
 *
 * Ability가 발사 정보를 초기화한 뒤 생성과 적중 대기 등록을 마치고 활성화한다.
 * 캐릭터는 겹침으로 검사하고 지형과 부딪히면 종료한다. 피해 계산은 HitResolver에 맡긴다.
 * 공격 실행 기록과 후속 효과의 소유권은 Combat에 있으며, 발사체는 적중·종료 사건만 알린다.
 */
UCLASS(Blueprintable)
class MAVERICK_API AMVProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    AMVProjectileBase();

    virtual void Tick(float DeltaSeconds) override;

    // SpawnActorDeferred 이후, FinishSpawning 이전에 호출한다.
    bool InitializeProjectile(const FMVProjectileLaunchContext& Context);

    // FinishSpawning과 Combat의 적중 대기 등록 이후에 호출한다.
    bool ActivateProjectile();

    bool IsHomingEnabled() const { return bEnableHoming; }
    const USphereComponent* GetTargetingCollision() const { return Collision.Get(); }

    int32 GetAttackInstanceId() const { return LaunchContext.AttackInstanceId; }

    UPROPERTY(BlueprintAssignable, Category = "Maverick|Projectile|Event")
    FMVOnProjectileHitSignature OnProjectileHit;

    UPROPERTY(BlueprintAssignable, Category = "Maverick|Projectile|Event")
    FMVOnProjectileFinishedSignature OnProjectileFinished;

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Projectile|Components")
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Projectile|Components")
    TObjectPtr<UStaticMeshComponent> VisualMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Projectile|Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile")
    bool bPierceCharacters = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile")
    bool bEnableHoming = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Projectile", meta = (ClampMin = "0.01"))
    float MaxLifeSeconds = 5.0f;

    UPROPERTY(Transient)
    FMVProjectileLaunchContext LaunchContext;

private:
    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleProjectileStop(const FHitResult& ImpactResult);

    bool CanHitCharacter(const AMVCharacterBase* Victim) const;
    void ClearHomingTarget();
    void FinishProjectile();

    TSet<TWeakObjectPtr<AMVCharacterBase>> HitCharacters;

    bool bInitialized = false;
    bool bActive = false;
    bool bEnding = false;
    bool bResolvingHit = false;
    bool bFinishNotified = false;
};
