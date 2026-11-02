#include "Weapon/Passive/Katana/MVWeakPointPassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"

void UMVWeakPointPassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle() || BoundCombatComponent.IsValid())
	{
		return;
	}

	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
	UMVCombatComponent* Combat = IsValid(Owner) ? Owner->CombatComponent.Get() : nullptr;
	if (!IsValid(Combat) || Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundCombatComponent = Combat;
	CriticalChanceDelegateHandle = Combat->OnModifyOutgoingCriticalChance.AddUObject(
		this, &UMVWeakPointPassiveBehavior::HandleCriticalChance);
}

void UMVWeakPointPassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombatComponent.Get())
	{
		if (CriticalChanceDelegateHandle.IsValid())
		{
			Combat->OnModifyOutgoingCriticalChance.Remove(CriticalChanceDelegateHandle);
		}
	}

	CriticalChanceDelegateHandle = FDelegateHandle();
	BoundCombatComponent.Reset();
}

void UMVWeakPointPassiveBehavior::HandleCriticalChance(
	const FMVResolvedHitData& HitData,
	float& InOutChance)
{
	UMVCombatComponent* Combat = BoundCombatComponent.Get();
	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
	AMVCharacterBase* Victim = HitData.Victim.Get();
	if (!IsValid(Combat)
		|| !IsValid(Owner)
		|| !IsValid(Victim)
		|| HitData.Attacker.Get() != Owner
		|| !Combat->IsAttackFromCurrentPassiveActivation(HitData)
		|| !FMath::IsFinite(InOutChance))
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

	UMVStatComponent* VictimStats = Victim->StatComponent.Get();
	if (!IsValid(VictimStats)
		|| VictimStats->IsDead()
		|| !FMath::IsFinite(VictimStats->MaxHP)
		|| !FMath::IsFinite(VictimStats->CurrentHP)
		|| VictimStats->MaxHP <= 0.0f
		|| VictimStats->CurrentHP <= 0.0f)
	{
		return;
	}

	const bool bLowHealth = VictimStats->CurrentHP
		<= VictimStats->MaxHP * FMath::Clamp(TargetHealthThresholdRatio, 0.0f, 1.0f);
	if (bLowHealth)
	{
		InOutChance = FMath::Clamp(
			InOutChance + FMath::Clamp(CriticalChanceBonus, 0.0f, 1.0f), 0.0f, 1.0f);
	}
}
