#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"
#include "MVConditionalIncomingDamageBehavior.generated.h"

class UMVStatComponent;

/**
 * 피격 직전의 체력 조건을 확인해 효과 소유자가 받는 체력 피해에 배율을 적용한다.
 * 효과 적용 시 피해 계산을 구독하고 제거 시 자기 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Conditional Incoming Damage"))
class MAVERICK_API UMVConditionalIncomingDamageBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConditionalIncomingDamage")
	FMVPassiveHealthCondition HealthCondition;

	/** 받는 피해에 곱할 값. 0.70은 30% 감소를 뜻한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConditionalIncomingDamage", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 0.70f;

private:
	void HandleIncomingDamage(const FMVResolvedHitData& HitData, float& InOutDamage);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	FDelegateHandle IncomingDamageDelegateHandle;
};
