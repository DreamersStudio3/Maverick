#include "Weapon/Passive/TwoHandedShield/MVIndomitableWillPassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"

void UMVIndomitableWillPassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
	UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;

	if (!IsValid(Stats) || Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundStatComponent = Stats;
	IncomingDamageDelegateHandle =
		Stats->OnModifyIncomingDamage.AddUObject(
			this,
			&UMVIndomitableWillPassiveBehavior::HandleIncomingDamage);
}

void UMVIndomitableWillPassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		if (IncomingDamageDelegateHandle.IsValid())
		{
			Stats->OnModifyIncomingDamage.Remove(IncomingDamageDelegateHandle);
		}
	}

	IncomingDamageDelegateHandle = FDelegateHandle();
	BoundStatComponent.Reset();
}

void UMVIndomitableWillPassiveBehavior::HandleIncomingDamage(
	const FMVResolvedHitData& HitData,
	float& InOutDamage)
{
	UMVStatComponent* Stats = BoundStatComponent.Get();
	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;

	if (!IsValid(Stats)
		|| !IsValid(Owner)
		|| Stats->GetOwner() != Owner
		|| Stats->IsDead()
		|| HitData.Victim.Get() != Owner
		|| Stats->MaxHP <= 0.0f
		|| Stats->CurrentHP <= 0.0f
		|| !FMath::IsFinite(InOutDamage)
		|| InOutDamage <= 0.0f)
	{
		return;
	}

	const FMVStatusEffectHandle LocalEffectHandle = GetEffectHandle();
	const FMVStatusEffectInstance* ActiveInstance =
		Effects->GetActiveEffects().FindByPredicate(
			[LocalEffectHandle](const FMVStatusEffectInstance& Instance)
			{
				return Instance.Handle == LocalEffectHandle;
			});

	if (!ActiveInstance
		|| !ActiveInstance->IsValid()
		|| ActiveInstance->SourceActor.Get() != Owner)
	{
		return;
	}

	const float Threshold = Stats->MaxHP * FMath::Clamp(HealthThresholdRatio, 0.0f, 1.0f);

	if (Stats->CurrentHP > Threshold)
	{
		return;
	}

	const float ReducedDamage = InOutDamage * (1.0f - FMath::Clamp(DamageReductionRatio, 0.0f, 1.0f));

	if (!FMath::IsFinite(ReducedDamage))
	{
		return;
	}

	InOutDamage = ReducedDamage;
}
