#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVAttackSpeedPerStackBehavior.generated.h"

class UMVStatComponent;

/**
 * 상태 효과 중첩 수를 소유 캐릭터의 공격속도 증가분으로 반영하는 행동.
 *
 * 최초 적용 시 보정을 등록하고 중첩 갱신 시 같은 보정을 변경한다.
 * 만료·해제 시 자신이 등록한 보정만 정리한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVAttackSpeedPerStackBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnUpdated_Implementation(
		const FMVStatusEffectInstance& Instance,
		int32 PreviousStacks) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

private:
	void ApplyStackBonus(const FMVStatusEffectInstance& Instance);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> ModifiedStatComponent;

	UPROPERTY(Transient)
	FGuid ModifierHandle;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AttackSpeed", meta = (ClampMin = "0.0"))
	float BonusRatioPerStack = 0.02f;
};
