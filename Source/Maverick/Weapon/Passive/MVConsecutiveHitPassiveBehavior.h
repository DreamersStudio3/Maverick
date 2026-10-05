#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVConsecutiveHitPassiveBehavior.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatusEffectDefinition;

/**
 * 무기 패시브가 적용된 동안 평타의 명중·빗나감과 소유자의 피격·사망을 감시한다.
 * 유효 명중마다 중첩 효과를 적용하고, 설정된 초기화 조건에서 제거한다.
 * 패시브가 제거되면 사건 구독과 자신이 만든 중첩 효과를 정리한다.
 */
UCLASS(BlueprintType, EditInlineNew)
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

	// 유연한 흐름은 true, 추후 아웃사이더 스텝은 false로 설정한다.
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
