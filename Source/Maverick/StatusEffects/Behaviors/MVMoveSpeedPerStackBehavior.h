#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVMoveSpeedPerStackBehavior.generated.h"

class UMVStatComponent;

/**
 * 상태 효과 중첩 수를 소유 캐릭터의 이동속도 증가율로 반영한다.
 * 적용·중첩 갱신 시 같은 보정을 갱신하고, 제거 시 자기 보정만 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVMoveSpeedPerStackBehavior : public UMVStatusEffectBehavior
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveSpeed", meta = (ClampMin = "0.0"))
	float BonusRatioPerStack = 0.01f;

private:
	void ApplyStackBonus(const FMVStatusEffectInstance& Instance);

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> ModifiedStatComponent;

	UPROPERTY(Transient)
	FGuid ModifierHandle;
};