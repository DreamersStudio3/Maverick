#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"
#include "MVTripleBoltPassiveBehavior.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatusEffectComponent;
class UMVStatusEffectDefinition;

/**
 * 트리플 볼트 본체가 활성화된 동안 피해 처리까지 끝난 평타 명중을 감시한다.
 * 살아 있는 대상에게 시전자별 표식을 1중첩씩 적용한다.
 * 본체 해제 또는 시전자·대상 사망 시 자신이 만든 표식을 제거하고 사건 구독을 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVTripleBoltPassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(
		const FMVStatusEffectInstance& Instance) override;

	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TripleBolt")
	TObjectPtr<UMVStatusEffectDefinition> MarkEffectDefinition = nullptr;

private:
	UFUNCTION()
	void HandleValidatedAttackHitAfterDamage(
		const FMVResolvedHitData& HitData,
		UMVAbilityBase* Ability);

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);

	UFUNCTION()
	void HandleMarkedTargetDeathStarted(const FMVDeathContext& DeathContext);

	void PruneInactiveTargets();
	void RemoveOwnedMarks();

	UPROPERTY(Transient)
	TWeakObjectPtr<AMVCharacterBase> BoundOwner;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVCombatComponent> BoundCombatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatusEffectDefinition> PassiveDefinition;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UMVStatusEffectComponent>> MarkedTargets;
};