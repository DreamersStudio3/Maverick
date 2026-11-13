#pragma once

#include "CoreMinimal.h"
#include "MVWeaponPassiveConditions.generated.h"

class AMVCharacterBase;
class UMVAbilityBase;
class UMVCombatComponent;
class UMVStatComponent;
class UMVStatusEffectBehavior;
struct FMVResolvedHitData;
struct FMVStatusEffectInstance;

UENUM(BlueprintType)
enum class EMVHitStackAttackFilter : uint8
{
	BasicAttackOnly,
	AnyValidatedAttack
};

UENUM(BlueprintType)
enum class EMVPassiveHealthSubject : uint8
{
	Owner,
	HitTarget
};

UENUM(BlueprintType)
enum class EMVPassiveHealthComparison : uint8
{
	AtMost,
	AtLeast
};

/** 보정 시점의 체력을 검사하는 설정. 런타임 상태는 보관하지 않는다. */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVPassiveHealthCondition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Condition")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Condition", meta = (EditCondition = "bEnabled"))
	EMVPassiveHealthSubject Subject = EMVPassiveHealthSubject::Owner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Condition", meta = (EditCondition = "bEnabled"))
	EMVPassiveHealthComparison Comparison = EMVPassiveHealthComparison::AtMost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Condition",
		meta = (EditCondition = "bEnabled", ClampMin = "0.0", ClampMax = "1.0"))
	float ThresholdRatio = 0.30f;

	bool Matches(const AMVCharacterBase& Owner, const FMVResolvedHitData& HitData) const;
};

namespace MVWeaponPassiveConditions
{
	AMVCharacterBase* ResolveOwner(const UMVStatusEffectBehavior& Behavior);
	const FMVStatusEffectInstance* FindActiveSelfAppliedEffect(
		const UMVStatusEffectBehavior& Behavior, const AMVCharacterBase& Owner);
	bool HasActiveSelfAppliedEffect(const UMVStatusEffectBehavior& Behavior, const AMVCharacterBase& Owner);
	bool MatchesAttackFilter(
		EMVHitStackAttackFilter Filter, const UMVCombatComponent& Combat, const UMVAbilityBase* Ability);
	bool CanModifyIncomingDamage(
		const UMVStatusEffectBehavior& Behavior, const UMVStatComponent* Stats,
		const AMVCharacterBase* Owner, const FMVResolvedHitData& HitData, float Damage);
	void ApplyFiniteDamageMultiplier(float Multiplier, float& InOutDamage);
}
