#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "Struct/MVHitTypes.h"

bool FMVPassiveHealthCondition::Matches(const AMVCharacterBase& Owner, const FMVResolvedHitData& HitData) const
{
	if (!bEnabled)
	{
		return true;
	}

	const AMVCharacterBase* SubjectCharacter =
		Subject == EMVPassiveHealthSubject::Owner ? &Owner : HitData.Victim.Get();
	const UMVStatComponent* Stats = IsValid(SubjectCharacter) ? SubjectCharacter->StatComponent.Get() : nullptr;
	if (!IsValid(Stats)
		|| Stats->IsDead()
		|| !FMath::IsFinite(Stats->MaxHP)
		|| !FMath::IsFinite(Stats->CurrentHP)
		|| Stats->MaxHP <= 0.0f
		|| Stats->CurrentHP <= 0.0f)
	{
		return false;
	}

	const float Threshold = Stats->MaxHP * FMath::Clamp(ThresholdRatio, 0.0f, 1.0f);
	return Comparison == EMVPassiveHealthComparison::AtMost
		? Stats->CurrentHP <= Threshold
		: Stats->CurrentHP >= Threshold;
}

AMVCharacterBase* MVWeaponPassiveConditions::ResolveOwner(const UMVStatusEffectBehavior& Behavior)
{
	const UMVStatusEffectComponent* Effects = Behavior.GetOwningStatusEffectComponent();
	return IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
}

const FMVStatusEffectInstance* MVWeaponPassiveConditions::FindActiveSelfAppliedEffect(
	const UMVStatusEffectBehavior& Behavior, const AMVCharacterBase& Owner)
{
	const UMVStatusEffectComponent* Effects = Behavior.GetOwningStatusEffectComponent();
	if (!IsValid(Effects))
	{
		return nullptr;
	}

	const FMVStatusEffectHandle Handle = Behavior.GetEffectHandle();
	const FMVStatusEffectInstance* Instance = Effects->GetActiveEffects().FindByPredicate(
		[Handle](const FMVStatusEffectInstance& Candidate)
		{
			return Candidate.Handle == Handle;
		});
	return Instance && Instance->IsValid() && Instance->SourceActor.Get() == &Owner ? Instance : nullptr;
}

bool MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(
	const UMVStatusEffectBehavior& Behavior, const AMVCharacterBase& Owner)
{
	return FindActiveSelfAppliedEffect(Behavior, Owner) != nullptr;
}

bool MVWeaponPassiveConditions::MatchesAttackFilter(
	EMVHitStackAttackFilter Filter, const UMVCombatComponent& Combat, const UMVAbilityBase* Ability)
{
	return Filter == EMVHitStackAttackFilter::AnyValidatedAttack || Combat.IsBasicAttackAbility(Ability);
}

bool MVWeaponPassiveConditions::CanModifyIncomingDamage(
	const UMVStatusEffectBehavior& Behavior, const UMVStatComponent* Stats,
	const AMVCharacterBase* Owner, const FMVResolvedHitData& HitData, float Damage)
{
	return IsValid(Stats)
		&& IsValid(Owner)
		&& Stats->GetOwner() == Owner
		&& !Stats->IsDead()
		&& Stats->CurrentHP > 0.0f
		&& HitData.Victim.Get() == Owner
		&& FMath::IsFinite(Damage)
		&& Damage > 0.0f
		&& HasActiveSelfAppliedEffect(Behavior, *Owner);
}

void MVWeaponPassiveConditions::ApplyFiniteDamageMultiplier(float Multiplier, float& InOutDamage)
{
	if (!FMath::IsFinite(Multiplier))
	{
		return;
	}

	const float ModifiedDamage = InOutDamage * FMath::Max(0.0f, Multiplier);
	if (FMath::IsFinite(ModifiedDamage))
	{
		InOutDamage = ModifiedDamage;
	}
}
