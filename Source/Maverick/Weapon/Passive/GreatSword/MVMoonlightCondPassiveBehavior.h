#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVMoonlightCondPassiveBehavior.generated.h"

class UMVAbilityBase;
class UMVCombatComponent;

/**
 * 월광의 응집이 적용된 동안 공격 피해 계산을 감시한다.
 *
 * 유효한 명중 5회로 상태 효과의 중첩을 1에서 6까지 올린다.
 * 중첩 6에서 다음 명중의 피해를 강화하고 중첩을 1로 되돌린다.
 * 효과가 제거되면 피해 계산 사건의 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVMoonlightCondPassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoonlightCondensation", meta = (ClampMin = "0.0"))
	float BonusDamageRatio = 0.20f;

private:
	void HandleOutgoingAttackDamage(const FMVResolvedHitData& HitData, float& InOutDamage);

	UFUNCTION()
	void HandleAttackMissed(UMVAbilityBase* MissedAbility);

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

	void ResetCharge();

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	FDelegateHandle OutgoingDamageDelegateHandle;
};
