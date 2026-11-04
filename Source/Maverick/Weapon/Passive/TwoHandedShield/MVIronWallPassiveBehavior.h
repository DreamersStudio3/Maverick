#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVIronWallPassiveBehavior.generated.h"

class UMVStatComponent;

/**
 * 철벽 본체 효과가 유지되는 동안 평타·스킬 방어력과 성장 포함 최대 체력을 높인다.
 * 적용 시 능력치 구성요소에 보정 두 개를 등록하고, 제거 시 각 고유 번호로 자기 보정만 해제한다.
 * 현재 체력 제한과 레벨업 미리보기 계산은 능력치 구성요소가 소유한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVIronWallPassiveBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance,
		EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IronWall", meta = (ClampMin = "0.0"))
	float DefenceBonusRatio = 0.20f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IronWall", meta = (ClampMin = "0.0"))
	float MaxHPBonusRatio = 0.20f;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> BoundStatComponent;

	FGuid DefenceModifierHandle;
	FGuid MaxHPModifierHandle;
};
