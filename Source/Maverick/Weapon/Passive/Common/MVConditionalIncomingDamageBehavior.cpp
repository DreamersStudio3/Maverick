#include "Weapon/Passive/Common/MVConditionalIncomingDamageBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Struct/MVHitTypes.h"

void UMVConditionalIncomingDamageBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle() || BoundStatComponent.IsValid())
	{
		return;
	}

	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;
	if (!IsValid(Stats) || Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundStatComponent = Stats;
	IncomingDamageDelegateHandle = Stats->OnModifyIncomingDamage.AddUObject(
		this, &UMVConditionalIncomingDamageBehavior::HandleIncomingDamage);
}

void UMVConditionalIncomingDamageBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/, EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVStatComponent* Stats = BoundStatComponent.Get(); Stats && IncomingDamageDelegateHandle.IsValid())
	{
		Stats->OnModifyIncomingDamage.Remove(IncomingDamageDelegateHandle);
	}

	IncomingDamageDelegateHandle.Reset();
	BoundStatComponent.Reset();
}

void UMVConditionalIncomingDamageBehavior::HandleIncomingDamage(const FMVResolvedHitData& HitData, float& InOutDamage)
{
	UMVStatComponent* Stats = BoundStatComponent.Get();
	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	if (!MVWeaponPassiveConditions::CanModifyIncomingDamage(*this, Stats, Owner, HitData, InOutDamage)
		|| !HealthCondition.Matches(*Owner, HitData))
	{
		return;
	}

	MVWeaponPassiveConditions::ApplyFiniteDamageMultiplier(DamageMultiplier, InOutDamage);
}
