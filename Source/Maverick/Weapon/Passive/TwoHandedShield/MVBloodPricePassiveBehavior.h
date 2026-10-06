#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVBloodPricePassiveBehavior.generated.h"

/**
 * 장착 중 실제 체력 피해를 최대 체력에 대한 비율로 누적한다.
 * 누적 비율이 5%에 도달할 때마다 발동 횟수를 계산하고 나머지를 보관한다.
 * 사망 또는 효과 제거 시 사건 구독과 남은 누적 비율을 정리한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVBloodPricePassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(
		const FMVStatusEffectInstance& Instance) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BloodPrice", meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float DamageThresholdRatio = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BloodPrice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CooldownRefundRatio = 0.10f;

private:
	UFUNCTION()
	void HandleDamageApplied(
		float AppliedDamage,
		float PreviousHP,
		float CurrentHP,
		const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	double AccumulatedDamageRatio = 0.0;
};