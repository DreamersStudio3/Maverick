#include "Weapon/Passive/MVConsecutiveHitPassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"

void UMVConsecutiveHitPassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle()
		|| !IsValid(StackEffectDefinition.Get()))
	{
		return;
	}

	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects)
		? Cast<AMVCharacterBase>(Effects->GetOwner())
		: nullptr;
	UMVCombatComponent* Combat = IsValid(Owner)
	? Owner->CombatComponent.Get()
	: nullptr;
	UMVStatComponent* Stats = IsValid(Owner)
		? Owner->StatComponent.Get()
		: nullptr;

	if (!IsValid(Combat)
		|| !IsValid(Stats)
		|| Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundCharacter = Owner;
	BoundCombatComponent = Combat;
	BoundStatComponent = Stats;
	PassiveDefinition = Instance.Definition.Get();

	Combat->OnValidatedAttackHit.AddUniqueDynamic(
		this,
		&UMVConsecutiveHitPassiveBehavior::HandleValidatedAttackHit);
	Combat->OnAttackMissed.AddUniqueDynamic(
		this,
		&UMVConsecutiveHitPassiveBehavior::HandleAttackMissed);
	Owner->OnDamaged.AddUniqueDynamic(
		this,
		&UMVConsecutiveHitPassiveBehavior::HandleOwnerDamaged);
	Stats->OnDeathStarted.AddUniqueDynamic(
		this,
		&UMVConsecutiveHitPassiveBehavior::HandleOwnerDeathStarted);
}

void UMVConsecutiveHitPassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombatComponent.Get())
	{
		Combat->OnValidatedAttackHit.RemoveDynamic(
			this,
			&UMVConsecutiveHitPassiveBehavior::HandleValidatedAttackHit);
		Combat->OnAttackMissed.RemoveDynamic(
			this,
			&UMVConsecutiveHitPassiveBehavior::HandleAttackMissed);
	}

	if (AMVCharacterBase* Owner = BoundCharacter.Get())
	{
		Owner->OnDamaged.RemoveDynamic(
			this,
			&UMVConsecutiveHitPassiveBehavior::HandleOwnerDamaged);
	}

	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		Stats->OnDeathStarted.RemoveDynamic(
			this,
			&UMVConsecutiveHitPassiveBehavior::HandleOwnerDeathStarted);
	}

	RemoveStackEffect(EMVStatusEffectRemovalReason::Manual);

	BoundCombatComponent.Reset();
	BoundStatComponent.Reset();
	BoundCharacter.Reset();
	PassiveDefinition.Reset();
}

void UMVConsecutiveHitPassiveBehavior::HandleValidatedAttackHit(
	const FMVResolvedHitData& HitData,
	UMVAbilityBase* Ability)
{
	AMVCharacterBase* Owner = BoundCharacter.Get();
	UMVCombatComponent* Combat = BoundCombatComponent.Get();
	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();

	if (!IsValid(Owner)
		|| !IsValid(Combat)
		|| !IsValid(Effects)
		|| !IsValid(Ability)
		|| !IsValid(StackEffectDefinition.Get())
		|| !IsValid(PassiveDefinition.Get())
		|| HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| HitData.Attacker.Get() != Owner
		|| Ability->GetOwnerCharacter() != Owner
		|| !Combat->IsBasicAttackAbility(Ability)
		|| Effects->FindStatusEffectHandle(
			PassiveDefinition.Get(), Owner) != GetEffectHandle())
	{
		return;
	}

	UMVStatComponent* Stats = Owner->StatComponent.Get();
	if (!IsValid(Stats) || Stats->IsDead())
	{
		return;
	}

	FMVStatusEffectSpec Spec;
	Spec.Definition = StackEffectDefinition;
	Spec.SourceActor = Owner;
	Spec.StackDelta = 1;

	Effects->ApplyStatusEffect(Spec);
}

void UMVConsecutiveHitPassiveBehavior::HandleAttackMissed(
	UMVAbilityBase* MissedAbility)
{
	AMVCharacterBase* Owner = BoundCharacter.Get();
	UMVCombatComponent* Combat = BoundCombatComponent.Get();

	if (!IsValid(Owner)
		|| !IsValid(Combat)
		|| !IsValid(MissedAbility)
		|| MissedAbility->GetOwnerCharacter() != Owner
		|| !Combat->IsBasicAttackAbility(MissedAbility))
	{
		return;
	}

	RemoveStackEffect(EMVStatusEffectRemovalReason::Consumed);
}

void UMVConsecutiveHitPassiveBehavior::HandleOwnerDamaged(
	const FMVResolvedHitData& HitData)
{
	if (bResetOnOwnerDamaged
		&& HitData.Victim.Get() == BoundCharacter.Get())
	{
		RemoveStackEffect(EMVStatusEffectRemovalReason::Consumed);
	}
}

void UMVConsecutiveHitPassiveBehavior::HandleOwnerDeathStarted(
	const FMVDeathContext& DeathContext)
{
	if (DeathContext.DeadActor.Get() == BoundCharacter.Get())
	{
		RemoveStackEffect(EMVStatusEffectRemovalReason::Consumed);
	}
}

void UMVConsecutiveHitPassiveBehavior::RemoveStackEffect(
	EMVStatusEffectRemovalReason RemovalReason)
{
	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = BoundCharacter.Get();

	if (!IsValid(Effects)
		|| !IsValid(Owner)
		|| !IsValid(StackEffectDefinition.Get()))
	{
		return;
	}

	const FMVStatusEffectHandle StackHandle =
		Effects->FindStatusEffectHandle(
			StackEffectDefinition.Get(), Owner);
	if (!StackHandle.IsValid())
	{
		return;
	}

	Effects->RemoveStatusEffect(StackHandle, RemovalReason);
}