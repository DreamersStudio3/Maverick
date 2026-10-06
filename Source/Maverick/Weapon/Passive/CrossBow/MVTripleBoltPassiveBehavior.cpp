#include "Weapon/Passive/CrossBow/MVTripleBoltPassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"

void UMVTripleBoltPassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle()
		|| !IsValid(MarkEffectDefinition.Get()))
	{
		return;
	}

	UMVStatusEffectComponent* OwnerEffects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(OwnerEffects)
		? Cast<AMVCharacterBase>(OwnerEffects->GetOwner())
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

	BoundOwner = Owner;
	BoundCombatComponent = Combat;
	BoundStatComponent = Stats;
	PassiveDefinition = Instance.Definition.Get();

	Combat->OnValidatedAttackHitAfterDamage.AddUniqueDynamic(
		this,
		&UMVTripleBoltPassiveBehavior::HandleValidatedAttackHitAfterDamage);
	Stats->OnDeathStarted.AddUniqueDynamic(
		this,
		&UMVTripleBoltPassiveBehavior::HandleOwnerDeathStarted);
}

void UMVTripleBoltPassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombatComponent.Get())
	{
		Combat->OnValidatedAttackHitAfterDamage.RemoveDynamic(
			this,
			&UMVTripleBoltPassiveBehavior::HandleValidatedAttackHitAfterDamage);
	}

	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		Stats->OnDeathStarted.RemoveDynamic(
			this,
			&UMVTripleBoltPassiveBehavior::HandleOwnerDeathStarted);
	}

	RemoveOwnedMarks();

	BoundOwner.Reset();
	BoundCombatComponent.Reset();
	BoundStatComponent.Reset();
	PassiveDefinition.Reset();
}

void UMVTripleBoltPassiveBehavior::HandleValidatedAttackHitAfterDamage(
	const FMVResolvedHitData& HitData,
	UMVAbilityBase* Ability)
{
	AMVCharacterBase* Owner = BoundOwner.Get();
	AMVCharacterBase* Victim = HitData.Victim.Get();
	UMVCombatComponent* Combat = BoundCombatComponent.Get();
	UMVStatusEffectComponent* OwnerEffects = GetOwningStatusEffectComponent();

	if (!IsValid(Owner)
		|| !IsValid(Victim)
		|| Victim == Owner
		|| !IsValid(Combat)
		|| !IsValid(OwnerEffects)
		|| !IsValid(Ability)
		|| !IsValid(MarkEffectDefinition.Get())
		|| !IsValid(PassiveDefinition.Get())
		|| HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| HitData.Attacker.Get() != Owner
		|| Ability->GetOwnerCharacter() != Owner
		|| !Combat->IsBasicAttackAbility(Ability)
		|| OwnerEffects->FindStatusEffectHandle(
			PassiveDefinition.Get(), Owner) != GetEffectHandle())
	{
		return;
	}

	PruneInactiveTargets();

	UMVStatComponent* OwnerStats = BoundStatComponent.Get();
	UMVStatComponent* VictimStats = Victim->FindComponentByClass<UMVStatComponent>();
	UMVStatusEffectComponent* VictimEffects = Victim->FindComponentByClass<UMVStatusEffectComponent>();

	if (!IsValid(OwnerStats)
		|| OwnerStats->IsDead()
		|| !IsValid(VictimStats)
		|| VictimStats->IsDead()
		|| VictimStats->CurrentHP <= 0.0f
		|| !IsValid(VictimEffects))
	{
		return;
	}

	FMVStatusEffectSpec Spec;
	Spec.Definition = MarkEffectDefinition;
	Spec.SourceActor = Owner;
	Spec.StackDelta = 1;

	const FMVStatusEffectHandle AppliedHandle = VictimEffects->ApplyStatusEffect(Spec);
	const bool bMarkActive = VictimEffects->HasStatusEffect(MarkEffectDefinition.Get(), Owner);

	if (AppliedHandle.IsValid() && bMarkActive)
	{
		MarkedTargets.AddUnique(
			TWeakObjectPtr<UMVStatusEffectComponent>(VictimEffects));

		VictimStats->OnDeathStarted.AddUniqueDynamic(
			this,
			&UMVTripleBoltPassiveBehavior::HandleMarkedTargetDeathStarted);
	}
	else if (!bMarkActive)
	{
		if (IsValid(VictimStats))
		{
			VictimStats->OnDeathStarted.RemoveDynamic(
				this,
				&UMVTripleBoltPassiveBehavior::HandleMarkedTargetDeathStarted);
		}

		MarkedTargets.RemoveAll(
			[VictimEffects](
				const TWeakObjectPtr<UMVStatusEffectComponent>& Target)
			{
				return !Target.IsValid()
					|| Target.Get() == VictimEffects;
			});
	}
}

void UMVTripleBoltPassiveBehavior::HandleOwnerDeathStarted(const FMVDeathContext& DeathContext)
{
	if (DeathContext.DeadActor.Get() == BoundOwner.Get())
	{
		RemoveOwnedMarks();
	}
}

void UMVTripleBoltPassiveBehavior::HandleMarkedTargetDeathStarted(
	const FMVDeathContext& DeathContext)
{
	AMVCharacterBase* DeadTarget = Cast<AMVCharacterBase>(DeathContext.DeadActor.Get());
	UMVStatusEffectComponent* TargetEffects = IsValid(DeadTarget)
		? DeadTarget->FindComponentByClass<UMVStatusEffectComponent>()
		: nullptr;

	if (!IsValid(TargetEffects)
		|| !MarkedTargets.ContainsByPredicate(
			[TargetEffects](
				const TWeakObjectPtr<UMVStatusEffectComponent>& Target)
			{
				return Target.Get() == TargetEffects;
			}))
	{
		return;
	}

	AMVCharacterBase* Owner = BoundOwner.Get();
	if (IsValid(Owner) && IsValid(MarkEffectDefinition.Get()))
	{
		const FMVStatusEffectHandle MarkHandle =
			TargetEffects->FindStatusEffectHandle(
				MarkEffectDefinition.Get(), Owner);

		if (MarkHandle.IsValid())
		{
			TargetEffects->RemoveStatusEffect(
				MarkHandle,
				EMVStatusEffectRemovalReason::Manual);
		}
	}

	if (UMVStatComponent* TargetStats = DeadTarget->FindComponentByClass<UMVStatComponent>())
	{
		TargetStats->OnDeathStarted.RemoveDynamic(
			this,
			&UMVTripleBoltPassiveBehavior::HandleMarkedTargetDeathStarted);
	}

	MarkedTargets.RemoveAll(
		[TargetEffects](
			const TWeakObjectPtr<UMVStatusEffectComponent>& Target)
		{
			return !Target.IsValid()
				|| Target.Get() == TargetEffects;
		});
}

void UMVTripleBoltPassiveBehavior::PruneInactiveTargets()
{
	AMVCharacterBase* Owner = BoundOwner.Get();
	UMVStatusEffectDefinition* MarkDefinition = MarkEffectDefinition.Get();

	for (int32 Index = MarkedTargets.Num() - 1; Index >= 0; --Index)
	{
		UMVStatusEffectComponent* TargetEffects = MarkedTargets[Index].Get();

		if (IsValid(TargetEffects)
			&& IsValid(Owner)
			&& IsValid(MarkDefinition)
			&& TargetEffects->HasStatusEffect(MarkDefinition, Owner))
		{
			continue;
		}

		AActor* TargetActor = IsValid(TargetEffects)
			? TargetEffects->GetOwner()
			: nullptr;

		if (AMVCharacterBase* TargetCharacter = Cast<AMVCharacterBase>(TargetActor))
		{
			if (UMVStatComponent* TargetStats =
				TargetCharacter->FindComponentByClass<UMVStatComponent>())
			{
				TargetStats->OnDeathStarted.RemoveDynamic(
					this,
					&UMVTripleBoltPassiveBehavior::HandleMarkedTargetDeathStarted);
			}
		}

		MarkedTargets.RemoveAt(Index);
	}
}

void UMVTripleBoltPassiveBehavior::RemoveOwnedMarks()
{
	AMVCharacterBase* Owner = BoundOwner.Get();
	UMVStatusEffectDefinition* MarkDefinition = MarkEffectDefinition.Get();

	for (const TWeakObjectPtr<UMVStatusEffectComponent>& Target : MarkedTargets)
	{
		UMVStatusEffectComponent* TargetEffects = Target.Get();
		if (!IsValid(TargetEffects))
		{
			continue;
		}

		if (AMVCharacterBase* TargetCharacter = Cast<AMVCharacterBase>(TargetEffects->GetOwner()))
		{
			if (UMVStatComponent* TargetStats = TargetCharacter->FindComponentByClass<UMVStatComponent>())
			{
				TargetStats->OnDeathStarted.RemoveDynamic(
					this,
					&UMVTripleBoltPassiveBehavior::HandleMarkedTargetDeathStarted);
			}
		}

		if (IsValid(Owner) && IsValid(MarkDefinition))
		{
			const FMVStatusEffectHandle MarkHandle =
				TargetEffects->FindStatusEffectHandle(
					MarkDefinition, Owner);
			if (MarkHandle.IsValid())
			{
				TargetEffects->RemoveStatusEffect(
					MarkHandle,
					EMVStatusEffectRemovalReason::Manual);
			}
		}
	}

	MarkedTargets.Reset();
}
