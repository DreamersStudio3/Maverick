#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVRefundSkillUseOnHitBehavior.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatComponent;

/**
 * 적에게 피해를 준 유효 스킬 명중마다 발동 확률을 판정한다.
 * Combat이 원래 스킬 사용의 주 쿨타임 환원을 승인하면 그 사용에서 지불한 MP를 돌려준다.
 * 효과 적용 중 피해 처리 후 명중을 구독하고 제거 시 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Refund Skill Use on Hit"))
class MAVERICK_API UMVRefundSkillUseOnHitBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

protected:
	/** 조건을 만족하는 명중 1회당 환원 발동 확률. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RefundSkillUseOnHit",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ActivationChance = 0.10f;

private:
	UFUNCTION()
	void HandleValidatedAttackHitAfterDamage(const FMVResolvedHitData& HitData, UMVAbilityBase* Ability);

	UPROPERTY(Transient)
	TWeakObjectPtr<AMVCharacterBase> BoundOwner;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;
};
