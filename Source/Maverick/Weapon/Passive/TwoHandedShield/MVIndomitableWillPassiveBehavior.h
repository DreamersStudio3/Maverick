#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVIndomitableWillPassiveBehavior.generated.h"

class UMVStatComponent;

/**
 * 불굴의 의지가 장착된 동안 자신의 체력이 기준 이하이면
 * 받는 체력 피해를 감소시키는 상태 효과 행동.
 *
 * 적용 시 받는 피해 보정 사건을 구독하고,
 * 제거 시 자신이 보관한 구독 핸들만 해제한다.
 * 발동 여부는 각 피해를 받기 직전의 현재 체력으로 판단한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVIndomitableWillPassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IndomitableWill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthThresholdRatio = 0.30f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IndomitableWill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageReductionRatio = 0.30f;

private:
	void HandleIncomingDamage(const FMVResolvedHitData& HitData, float& InOutDamage);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	FDelegateHandle IncomingDamageDelegateHandle;
};