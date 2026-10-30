#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVGiantFortitudePassiveBehavior.generated.h"

class UMVStatComponent;

/**
 * 거인의 강인함 본체 효과가 유지되는 동안 기본·공격 중 강인도를 높이고 받는 체력 피해를 늘린다.
 * 적용 시 강인도 보정과 받는 피해 사건을 등록하고, 제거 시 자신의 핸들만 해제한다.
 * 강인도 소모·회복과 보스의 별도 강인도 구간은 능력치 구성요소가 관리한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVGiantFortitudePassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GiantFortitude", meta = (ClampMin = "0.0"))
	float PoiseBonusRatio = 0.30f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GiantFortitude", meta = (ClampMin = "0.0"))
	float IncomingDamageBonusRatio = 0.15f;

private:
	void HandleIncomingDamage(const FMVResolvedHitData& HitData, float& InOutDamage);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	FGuid PoiseModifierHandle;
	FDelegateHandle IncomingDamageDelegateHandle;
};
