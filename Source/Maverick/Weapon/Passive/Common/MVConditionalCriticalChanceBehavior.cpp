#include "Weapon/Passive/Common/MVConditionalCriticalChanceBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Struct/MVHitTypes.h"

UMVConditionalCriticalChanceBehavior::UMVConditionalCriticalChanceBehavior()
{
	HealthCondition.Subject = EMVPassiveHealthSubject::HitTarget;
}

void UMVConditionalCriticalChanceBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle() || BoundCombatComponent.IsValid())
	{
		return;
	}

	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	UMVCombatComponent* Combat = IsValid(Owner) ? Owner->CombatComponent.Get() : nullptr;
	if (!IsValid(Combat) || Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundCombatComponent = Combat;
	CriticalChanceDelegateHandle = Combat->OnModifyOutgoingCriticalChance.AddUObject(
		this, &UMVConditionalCriticalChanceBehavior::HandleCriticalChance);
}

void UMVConditionalCriticalChanceBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/, EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombatComponent.Get(); Combat && CriticalChanceDelegateHandle.IsValid())
	{
		Combat->OnModifyOutgoingCriticalChance.Remove(CriticalChanceDelegateHandle);
	}

	CriticalChanceDelegateHandle.Reset();
	BoundCombatComponent.Reset();
}

void UMVConditionalCriticalChanceBehavior::HandleCriticalChance(const FMVResolvedHitData& HitData, float& InOutChance)
{
	UMVCombatComponent* Combat = BoundCombatComponent.Get();
	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	if (!IsValid(Combat)
		|| !IsValid(Owner)
		|| !IsValid(HitData.Victim.Get())
		|| HitData.Attacker.Get() != Owner
		|| !Combat->IsAttackFromCurrentPassiveActivation(HitData)
		|| !FMath::IsFinite(InOutChance)
		|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(*this, *Owner)
		|| !HealthCondition.Matches(*Owner, HitData))
	{
		return;
	}

	InOutChance = FMath::Clamp(InOutChance + FMath::Clamp(BonusChance, 0.0f, 1.0f), 0.0f, 1.0f);
}
