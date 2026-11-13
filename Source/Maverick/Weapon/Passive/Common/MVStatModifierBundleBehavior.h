#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVStatModifierBundleBehavior.generated.h"

class UMVCombatComponent;
class UMVStatComponent;
struct FMVResolvedHitData;

UENUM(BlueprintType)
enum class EMVPassiveStatModifierKind : uint8
{
	Defence,
	MaxHP UMETA(DisplayName = "Max HP"),
	Poise,
	DodgeStaminaCost
};

UENUM(BlueprintType)
enum class EMVPassiveBundleDamageMode : uint8
{
	None,
	Incoming,
	OutgoingAttack
};

/** 장착 중 능력치 하나에 등록할 보정 설정. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVPassiveStatModifier
{
	GENERATED_BODY()

	FMVPassiveStatModifier() = default;
	FMVPassiveStatModifier(EMVPassiveStatModifierKind InKind, float InBonusRatio)
		: Kind(InKind), BonusRatio(InBonusRatio)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Modifier")
	EMVPassiveStatModifierKind Kind = EMVPassiveStatModifierKind::Defence;

	/** 0.20은 기준 능력치의 20%를 더한다는 뜻이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Modifier")
	float BonusRatio = 0.0f;
};

/**
 * 실행 중인 행동이 등록한 능력치 보정의 핸들을 보관한다.
 * 적용 실패 또는 행동 제거 시 자기 핸들을 등록의 역순으로 해제한다.
 */
class FMVRegisteredPassiveStatModifiers
{
public:
	bool TryApply(
		const UMVStatusEffectBehavior& Behavior,
		const FMVStatusEffectInstance& Instance,
		TArrayView<const FMVPassiveStatModifier> Modifiers);

	void Reset();

private:
	struct FRegisteredModifier
	{
		EMVPassiveStatModifierKind Kind;
		FGuid Handle;
	};

	TWeakObjectPtr<UMVStatComponent> BoundStats;
	TArray<FRegisteredModifier> Registered;
	bool bApplying = false;
};

/**
 * 효과 적용 시 설정된 능력치 보정을 순서대로 등록한다.
 * 모든 등록이 성공하면 선택한 피해 보정을 구독한다.
 * 등록 실패 시 등록분을 정리하고, 효과가 제거되면 피해 구독부터 해제한다.
 */
UCLASS(BlueprintType, EditInlineNew, meta = (DisplayName = "Stat Modifier Bundle"))
class MAVERICK_API UMVStatModifierBundleBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;
	virtual void OnRemoved_Implementation(
		const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Modifier Bundle")
	TArray<FMVPassiveStatModifier> Modifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Modifier Bundle")
	EMVPassiveBundleDamageMode DamageMode = EMVPassiveBundleDamageMode::None;

	/** 1.15는 해당 피해를 15% 높인다는 뜻이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Modifier Bundle",
		meta = (ClampMin = "0.0", EditCondition = "DamageMode != EMVPassiveBundleDamageMode::None"))
	float DamageMultiplier = 1.0f;

private:
	void HandleIncomingDamage(const FMVResolvedHitData& HitData, float& InOutDamage);
	void HandleOutgoingAttackDamage(const FMVResolvedHitData& HitData, float& InOutDamage);

	FMVRegisteredPassiveStatModifiers RegisteredModifiers;
	TWeakObjectPtr<UMVStatComponent> IncomingDamageSource;
	TWeakObjectPtr<UMVCombatComponent> OutgoingDamageSource;
	FDelegateHandle DamageDelegateHandle;
};
