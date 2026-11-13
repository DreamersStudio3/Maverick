#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVChargedHitDamageBehavior.generated.h"

class UMVAbilityBase;
class UMVCombatComponent;

/**
 * 유효 명중마다 이 효과의 중첩을 올리고 최대 중첩에서 다음 명중의 피해를 높인다.
 * 중첩 1이 빈 충전이며, 강화 타격 또는 초기화 시 중첩을 1로 되돌린다.
 * 효과 적용 중 공격 피해 계산과 초기화 사건을 구독하고 제거 시 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Boost Next Hit After Charging"))
class MAVERICK_API UMVChargedHitDamageBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedHitDamage", meta = (ClampMin = "0.0"))
	float ChargedHitBonusRatio = 0.20f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChargedHitDamage")
	bool bResetOnAttackMissed = true;

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
