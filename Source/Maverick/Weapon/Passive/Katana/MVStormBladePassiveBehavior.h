#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "Components/MVStatComponent.h"
#include "MVStormBladePassiveBehavior.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatusEffectDefinition;
class UMVStatComponent;

/**
 * 폭풍 칼날 패시브 본체가 살아 있는 동안 전투 사건을 감시한다.
 * 유효한 공격 명중마다 속도 효과를 중첩하고,
 * 빗나감·피격·패시브 해제 시 속도 효과를 제거한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVStormBladePassiveBehavior : public UMVStatusEffectBehavior
{
    GENERATED_BODY()

public:
    virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

    virtual void OnRemoved_Implementation(
        const FMVStatusEffectInstance& Instance,
        EMVStatusEffectRemovalReason RemovalReason) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StormBlade")
    TObjectPtr<UMVStatusEffectDefinition> SpeedEffectDefinition = nullptr;

private:
    UFUNCTION()
    void HandleValidatedAttackHit(const FMVResolvedHitData& HitData, UMVAbilityBase* Ability);

    UFUNCTION()
    void HandleAttackMissed(UMVAbilityBase* MissedAbility);

    UFUNCTION()
    void HandleOwnerDamaged(const FMVResolvedHitData& HitData);

    UFUNCTION()
    void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

    void RemoveSpeedEffect(EMVStatusEffectRemovalReason RemovalReason);

    UPROPERTY(Transient)
    TWeakObjectPtr<AMVCharacterBase> BoundCharacter;

    UPROPERTY(Transient)
    TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

    UPROPERTY(Transient)
    TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

    UPROPERTY(Transient)
    TWeakObjectPtr<UMVStatusEffectDefinition> PassiveDefinition;
};
