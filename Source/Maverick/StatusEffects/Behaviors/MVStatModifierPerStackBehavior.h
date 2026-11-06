#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVStatModifierPerStackBehavior.generated.h"

class UMVStatComponent;

UENUM(BlueprintType)
enum class EMVPerStackStat : uint8
{
	AttackSpeed UMETA(DisplayName = "Attack Speed"),
	MoveSpeed UMETA(DisplayName = "Move Speed")
};

/**
 * 효과를 받은 액터의 능력치 하나에 중첩당 증가율을 적용한다.
 * 최초 적용 시 보정을 등록하고, 재적용 시 같은 보정을 갱신하며, 제거 시 자기 보정만 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Stat Modifier Per Stack"))
class MAVERICK_API UMVStatModifierPerStackBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnUpdated_Implementation(const FMVStatusEffectInstance& Instance, int32 PreviousStacks) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Modifier")
	EMVPerStackStat Stat = EMVPerStackStat::AttackSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Modifier", meta = (ClampMin = "0.0"))
	float BonusRatioPerStack = 0.02f;

private:
	void ApplyStackBonus(const FMVStatusEffectInstance& Instance);
	void RemoveModifier();

	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatComponent> ModifiedStatComponent;

	UPROPERTY(Transient)
	FGuid ModifierHandle;
};
