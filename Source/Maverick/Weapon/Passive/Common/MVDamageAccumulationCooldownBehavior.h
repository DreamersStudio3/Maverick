#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVDamageAccumulationCooldownBehavior.generated.h"

/**
 * 실제 체력 피해를 피해 시점의 최대 체력에 대한 비율로 누적한다.
 * 기준에 도달한 횟수만큼 진행 중인 스킬 주 쿨타임을 줄이고 잔여 비율을 보관한다.
 * 사망 시 잔여량을 비우고, 효과 제거 시 피해·사망 사건 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Reduce Cooldowns on Damage Threshold"))
class MAVERICK_API UMVDamageAccumulationCooldownBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

protected:
	/** 발동 1회에 필요한 최대 체력 대비 누적 피해 비율. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DamageAccumulationCooldown",
		meta = (ClampMin = "0.001", ClampMax = "1.0"))
	float AccumulatedDamageThresholdRatio = 0.05f;

	/** 발동 1회에 줄일 주 쿨타임의 남은 시간 비율. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DamageAccumulationCooldown",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CooldownReductionRatio = 0.10f;

private:
	UFUNCTION()
	void HandleDamageApplied(float AppliedDamage, float PreviousHP, float CurrentHP, const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	double AccumulatedDamageRatio = 0.0;
};
