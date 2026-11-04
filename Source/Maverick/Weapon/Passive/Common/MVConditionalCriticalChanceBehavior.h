#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"
#include "MVConditionalCriticalChanceBehavior.generated.h"

class UMVCombatComponent;

/**
 * 현재 무기 패시브의 공격이 체력 조건을 만족할 때 이번 타격의 치명타 확률을 보정한다.
 * 효과 적용 시 치명타 계산을 구독하고 제거 시 자기 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Conditional Critical Chance"))
class MAVERICK_API UMVConditionalCriticalChanceBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	UMVConditionalCriticalChanceBehavior();

	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConditionalCriticalChance")
	FMVPassiveHealthCondition HealthCondition;

	/** 치명타 확률에 더할 값. 0.20은 20%포인트 증가를 뜻한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ConditionalCriticalChance",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BonusChance = 0.20f;

private:
	void HandleCriticalChance(const FMVResolvedHitData& HitData, float& InOutChance);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	FDelegateHandle CriticalChanceDelegateHandle;
};
