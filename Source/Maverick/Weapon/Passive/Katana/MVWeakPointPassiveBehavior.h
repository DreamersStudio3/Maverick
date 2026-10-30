#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVWeakPointPassiveBehavior.generated.h"

class UMVCombatComponent;

/**
 * 약점 포착 본체 효과의 치명타 확률 사건 구독을 소유한다.
 * 현재 패시브에서 나온 유효 공격이 체력 30% 이하의 대상에게 맞으면
 * 이번 타격의 치명타 확률에 20%포인트를 더하고, 제거 시 자신의 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVWeakPointPassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeakPoint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TargetHealthThresholdRatio = 0.30f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeakPoint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CriticalChanceBonus = 0.20f;

private:
	void HandleCriticalChance(const FMVResolvedHitData& HitData, float& InOutChance);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	FDelegateHandle CriticalChanceDelegateHandle;
};
