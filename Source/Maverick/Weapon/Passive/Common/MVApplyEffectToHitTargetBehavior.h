#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"
#include "MVApplyEffectToHitTargetBehavior.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatusEffectComponent;
class UMVStatusEffectDefinition;

/**
 * 피해 처리 후 지정한 공격에 맞고 살아 있는 대상에 시전자별 효과를 적용한다.
 * 적용한 효과를 대상별 핸들로 추적하고 대상·시전자 사망 또는 본체 제거 시 정리한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Apply Effect to Hit Target"))
class MAVERICK_API UMVApplyEffectToHitTargetBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitTargetEffect")
	EMVHitStackAttackFilter AttackFilter = EMVHitStackAttackFilter::BasicAttackOnly;

	/** 대상에게 적용할 효과. Instance Scope는 One Per Source로 설정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitTargetEffect")
	TObjectPtr<UMVStatusEffectDefinition> TargetEffectDefinition = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitTargetEffect", meta = (ClampMin = "1"))
	int32 StackDelta = 1;

private:
	UFUNCTION()
	void HandleValidatedAttackHitAfterDamage(const FMVResolvedHitData& HitData, UMVAbilityBase* Ability);

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

	UFUNCTION()
	void HandleTargetDeathStarted(const FMVDeathContext& DeathContext);

	void UnbindTargetDeath(UMVStatusEffectComponent* TargetEffects);
	void PruneInactiveTargets(const AMVCharacterBase& Owner);
	void RemoveTrackedEffects();

	UPROPERTY(Transient)
	TWeakObjectPtr<AMVCharacterBase> BoundOwner;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombat;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStats;

	TMap<TWeakObjectPtr<UMVStatusEffectComponent>, FMVStatusEffectHandle> TrackedEffects;
};
