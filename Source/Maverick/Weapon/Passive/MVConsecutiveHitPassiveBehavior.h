#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"
#include "MVConsecutiveHitPassiveBehavior.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatusEffectDefinition;

/**
 * 무기 패시브가 적용된 동안 지정한 공격의 유효 명중마다 별도 효과를 1중첩 적용한다.
 * 설정에 따라 빗나감·피격 시 중첩을 제거하고, 사망·패시브 해제 시에도 중첩을 정리한다.
 * 패시브가 제거되면 전투 사건 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Apply Stack Effect on Hit"))
class MAVERICK_API UMVConsecutiveHitPassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConsecutiveHit")
	TObjectPtr<UMVStatusEffectDefinition> StackEffectDefinition = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConsecutiveHit")
	EMVHitStackAttackFilter AttackFilter = EMVHitStackAttackFilter::BasicAttackOnly;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConsecutiveHit")
	bool bResetOnAttackMissed = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConsecutiveHit")
	bool bResetOnOwnerDamaged = true;

private:
	UFUNCTION()
	void HandleValidatedAttackHit(
		const FMVResolvedHitData& HitData,
		UMVAbilityBase* Ability);

	UFUNCTION()
	void HandleAttackMissed(UMVAbilityBase* MissedAbility);

	UFUNCTION()
	void HandleOwnerDamaged(const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

	void RemoveStackEffect(EMVStatusEffectRemovalReason RemovalReason);

	UPROPERTY(Transient)
	TWeakObjectPtr<AMVCharacterBase> BoundCharacter;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatusEffectDefinition> PassiveDefinition;
};
