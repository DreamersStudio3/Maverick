#include "Weapon/Passive/GreatSword/MVGiantFortitudePassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"

void UMVGiantFortitudePassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle() || BoundStatComponent.IsValid())
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

	const FGuid NewPoiseHandle = Stats->AddPoiseModifier(PoiseBonusRatio);
	if (!NewPoiseHandle.IsValid())
	{
		return;
	}

	BoundStatComponent = Stats;
	PoiseModifierHandle = NewPoiseHandle;
	IncomingDamageDelegateHandle = Stats->OnModifyIncomingDamage.AddUObject(
		this, &UMVGiantFortitudePassiveBehavior::HandleIncomingDamage);
}

void UMVGiantFortitudePassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		if (IncomingDamageDelegateHandle.IsValid())
		{
			Stats->OnModifyIncomingDamage.Remove(IncomingDamageDelegateHandle);
		}
		if (PoiseModifierHandle.IsValid())
		{
			Stats->RemovePoiseModifier(PoiseModifierHandle);
		}
	}

	IncomingDamageDelegateHandle = FDelegateHandle();
	PoiseModifierHandle = FGuid();
	BoundStatComponent.Reset();
}

void UMVGiantFortitudePassiveBehavior::HandleIncomingDamage(
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
		|| Stats->CurrentHP <= 0.0f
		|| HitData.Victim.Get() != Owner
		|| !FMath::IsFinite(InOutDamage)
		|| InOutDamage <= 0.0f)
	{
		return;
	}

	const FMVStatusEffectHandle ActiveHandle = GetEffectHandle();
	const FMVStatusEffectInstance* ActiveInstance = Effects->GetActiveEffects().FindByPredicate(
		[ActiveHandle](const FMVStatusEffectInstance& Candidate)
		{
			return Candidate.Handle == ActiveHandle;
		});
	if (!ActiveInstance
		|| !ActiveInstance->IsValid()
		|| ActiveInstance->SourceActor.Get() != Owner)
	{
		return;
	}

	const float ModifiedDamage = InOutDamage * (1.0f + FMath::Max(0.0f, IncomingDamageBonusRatio));
	if (FMath::IsFinite(ModifiedDamage))
	{
		InOutDamage = ModifiedDamage;
	}
}
