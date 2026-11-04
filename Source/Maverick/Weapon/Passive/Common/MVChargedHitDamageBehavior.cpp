#include "Weapon/Passive/Common/MVChargedHitDamageBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"

void UMVChargedHitDamageBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle() || BoundCombatComponent.IsValid())
	{
		return;
	}

	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	UMVCombatComponent* Combat = IsValid(Owner) ? Owner->CombatComponent.Get() : nullptr;
	UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;
	if (!IsValid(Combat) || !IsValid(Stats) || Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundCombatComponent = Combat;
	BoundStatComponent = Stats;
	OutgoingDamageDelegateHandle = Combat->OnModifyOutgoingAttackDamage.AddUObject(
		this, &UMVChargedHitDamageBehavior::HandleOutgoingAttackDamage);
	Combat->OnAttackMissed.AddUniqueDynamic(this, &UMVChargedHitDamageBehavior::HandleAttackMissed);
	Stats->OnDeathStarted.AddUniqueDynamic(this, &UMVChargedHitDamageBehavior::HandleOwnerDeathStarted);
}

void UMVChargedHitDamageBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/, EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombatComponent.Get())
	{
		Combat->OnModifyOutgoingAttackDamage.Remove(OutgoingDamageDelegateHandle);
		Combat->OnAttackMissed.RemoveDynamic(this, &UMVChargedHitDamageBehavior::HandleAttackMissed);
	}

	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		Stats->OnDeathStarted.RemoveDynamic(this, &UMVChargedHitDamageBehavior::HandleOwnerDeathStarted);
	}

	OutgoingDamageDelegateHandle.Reset();
	BoundCombatComponent.Reset();
	BoundStatComponent.Reset();
}

void UMVChargedHitDamageBehavior::HandleOutgoingAttackDamage(
	const FMVResolvedHitData& HitData, float& InOutDamage)
{
	const AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	const UMVCombatComponent* Combat = BoundCombatComponent.Get();
	if (!IsValid(Owner)
		|| !IsValid(Combat)
		|| HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| HitData.Attacker.Get() != Owner
		|| !IsValid(HitData.Victim.Get())
		|| !Combat->IsAttackFromCurrentPassiveActivation(HitData))
	{
		return;
	}

	const UMVStatComponent* Stats = Owner->StatComponent.Get();
	if (!IsValid(Stats) || Stats->IsDead())
	{
		return;
	}

	const FMVStatusEffectInstance* ActiveInstance = MVWeaponPassiveConditions::FindActiveSelfAppliedEffect(*this, *Owner);
	const UMVStatusEffectDefinition* Definition = ActiveInstance ? ActiveInstance->Definition.Get() : nullptr;
	if (!IsValid(Definition)
		|| Definition->StackPolicy != EMVStatusEffectStackPolicy::AddStack
		|| Definition->MaxStacks < 2)
	{
		return;
	}

	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	const FMVStatusEffectHandle Handle = ActiveInstance->Handle;
	const int32 StacksBefore = ActiveInstance->CurrentStacks;
	if (StacksBefore < Definition->MaxStacks)
	{
		Effects->SetStatusEffectStacks(Handle, StacksBefore + 1);
		return;
	}

	const float BoostedDamage = InOutDamage * (1.0f + FMath::Max(0.0f, ChargedHitBonusRatio));
	if (FMath::IsFinite(BoostedDamage) && Effects->SetStatusEffectStacks(Handle, 1))
	{
		InOutDamage = BoostedDamage;
	}
}

void UMVChargedHitDamageBehavior::HandleAttackMissed(UMVAbilityBase* MissedAbility)
{
	const AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	const UMVCombatComponent* Combat = BoundCombatComponent.Get();
	if (bResetOnAttackMissed
		&& IsValid(Owner)
		&& IsValid(Combat)
		&& IsValid(MissedAbility)
		&& MissedAbility->GetOwnerCharacter() == Owner
		&& Combat->IsAttackFromCurrentPassiveActivation(MissedAbility))
	{
		ResetCharge();
	}
}

void UMVChargedHitDamageBehavior::HandleOwnerDeathStarted(const FMVDeathContext& DeathContext)
{
	const AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	if (IsValid(Owner) && DeathContext.DeadActor.Get() == Owner)
	{
		ResetCharge();
	}
}

void UMVChargedHitDamageBehavior::ResetCharge()
{
	const AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	const FMVStatusEffectInstance* ActiveInstance = IsValid(Owner)
		? MVWeaponPassiveConditions::FindActiveSelfAppliedEffect(*this, *Owner)
		: nullptr;
	if (ActiveInstance && ActiveInstance->CurrentStacks > 1)
	{
		GetOwningStatusEffectComponent()->SetStatusEffectStacks(ActiveInstance->Handle, 1);
	}
}
