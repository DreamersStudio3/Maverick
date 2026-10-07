#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVPredatorCyclePassiveBehavior.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatusEffectDefinition;

/**
 * 장착 중 피해 처리가 끝난 스킬 명중마다 발동 확률을 판정한다.
 * 성공한 실행의 소비 마나와 주 쿨타임을 최대 한 번 환원한다.
 * 사망 또는 효과 제거 시 실행 기록과 사건 구독을 정리한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVPredatorCyclePassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PredatorCycle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RefundChance = 0.10f;

private:
	UFUNCTION()
	void HandleValidatedAttackHitAfterDamage(
		const FMVResolvedHitData& HitData,
		UMVAbilityBase* Ability);

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

	UPROPERTY(Transient)
	TWeakObjectPtr<AMVCharacterBase> BoundOwner;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatusEffectDefinition> PassiveDefinition;

	int32 RefundedAttackInstanceId = INDEX_NONE;
};