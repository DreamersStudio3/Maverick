#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVStackMaxHealthDamageBehavior.generated.h"

/**
* 상태 효과가 지정된 스택에 처음 도달했을 때
 * 대상 최대 체력 비례 피해를 적용하는 행동
 *
 * 선택적으로 다른 상태 효과가 존재하는 동안 발동을 차단
 * 피해 적용 성공 후 후속 상태 효과 적용 가능
 * 설정에 따라 소유 상태 효과를 소비하거나 유지
 *
 * 최초 적용 StackDelta가 임계값 이상인 경우 OnApplied에서 발동
 * 재적용으로 임계값에 도달한 경우 OnUpdated에서 발동
 */
class UMVStatusEffectDefinition;

UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVStackMaxHealthDamageBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnUpdated_Implementation(const FMVStatusEffectInstance& Instance, int32 PreviousStacks) override;

private:
	bool TryTriggerDamage(const FMVStatusEffectInstance& Instance, int32 PreviousStacks);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StackDamage", meta = (ClampMin = "1"))
	int32 RequiredStacks = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StackDamage", meta = (ClampMin = "0.0"))
	float TargetMaxHealthDamageRatio = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StackDamage|Trigger")
	TObjectPtr<UMVStatusEffectDefinition> TriggerBlockingEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StackDamage|Trigger")
	TObjectPtr<UMVStatusEffectDefinition> StatusEffectAppliedOnTrigger = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StackDamage|Trigger")
	bool bKeepOwningEffectAfterTrigger = false;
};