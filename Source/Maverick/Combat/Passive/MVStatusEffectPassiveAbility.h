#pragma once

#include "Combat/MVAbilityBase.h"
#include "Struct/MVStatusEffectTypes.h"

#include "MVStatusEffectPassiveAbility.generated.h"

class UMVStatusEffectComponent;
class UMVStatusEffectDefinition;

/**
 * 선택한 무기 패시브 본체 상태 효과 한 개의 수명을 CombatComponent의 Ability 슬롯에 연결한다.
 * 자식 Blueprint는 PassiveDefinition만 지정하고, 개별 발동 규칙은 기존 StatusEffectBehavior가 소유한다.
 * StartAbility는 중복 적용을 막고 자신이 새로 만든 핸들만 보관한다.
 * EndAbility와 외부 효과 제거 시 핸들·구독을 정리해 다른 경로의 효과를 지우지 않는다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVStatusEffectPassiveAbility : public UMVAbilityBase
{
	GENERATED_BODY()

public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	virtual void EndAbility_Implementation() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Combat|Passive")
	TObjectPtr<UMVStatusEffectDefinition> PassiveDefinition;

private:
	UFUNCTION()
	void HandleOwnedStatusEffectRemoved(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason);

	UPROPERTY(Transient)
	FMVStatusEffectHandle ActiveEffectHandle;

	TWeakObjectPtr<UMVStatusEffectComponent> AppliedEffectComponent;
};
