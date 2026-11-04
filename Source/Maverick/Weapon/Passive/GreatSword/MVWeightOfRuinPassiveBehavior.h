#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVWeightOfRuinPassiveBehavior.generated.h"

class UMVCombatComponent;
class UMVStatComponent;

/**
 * 파멸의 무게가 장착된 동안 유효한 공격 피해를 15% 증가시키고
 * 회피 스태미너 비용 증가율 20%를 등록한다.
 *
 * 적용 시 피해 보정 사건과 회피 비용 보정을 등록하고,
 * 제거 시 자신이 보관한 두 핸들로 각각 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVWeightOfRuinPassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeightOfRuin", meta = (ClampMin = "0.0"))
	float BonusDamageRatio = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeightOfRuin", meta = (ClampMin = "0.0"))
	float DodgeStaminaCostIncreaseRatio = 0.20f;

private:
	void HandleOutgoingAttackDamage(const FMVResolvedHitData& HitData, float& InOutDamage);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	FDelegateHandle OutgoingDamageDelegateHandle;
	FGuid DodgeStaminaCostModifierHandle;
};