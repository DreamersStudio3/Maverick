#include "Weapon/Passive/Common/MVApplyEffectToHitTargetBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "Struct/MVHitTypes.h"

void UMVApplyEffectToHitTargetBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle()
		|| !IsValid(TargetEffectDefinition.Get())
		|| TargetEffectDefinition->InstanceScope != EMVStatusEffectInstanceScope::OnePerSource)
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

	BoundOwner = Owner;
	BoundCombat = Combat;
	BoundStats = Stats;
	Combat->OnValidatedAttackHitAfterDamage.AddUniqueDynamic(
		this, &UMVApplyEffectToHitTargetBehavior::HandleValidatedAttackHitAfterDamage);
	Stats->OnDeathStarted.AddUniqueDynamic(this, &UMVApplyEffectToHitTargetBehavior::HandleOwnerDeathStarted);
}

void UMVApplyEffectToHitTargetBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/, EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombat.Get())
	{
		Combat->OnValidatedAttackHitAfterDamage.RemoveDynamic(
			this, &UMVApplyEffectToHitTargetBehavior::HandleValidatedAttackHitAfterDamage);
	}

	if (UMVStatComponent* Stats = BoundStats.Get())
	{
		Stats->OnDeathStarted.RemoveDynamic(this, &UMVApplyEffectToHitTargetBehavior::HandleOwnerDeathStarted);
	}

	BoundCombat.Reset();
	BoundStats.Reset();
	BoundOwner.Reset();
	RemoveTrackedEffects();
}

void UMVApplyEffectToHitTargetBehavior::HandleValidatedAttackHitAfterDamage(
	const FMVResolvedHitData& HitData, UMVAbilityBase* Ability)
{
	AMVCharacterBase* Owner = BoundOwner.Get();
	AMVCharacterBase* Victim = HitData.Victim.Get();
	UMVCombatComponent* Combat = BoundCombat.Get();
	UMVStatComponent* OwnerStats = BoundStats.Get();
	if (!IsValid(Owner)
		|| !IsValid(Victim)
		|| Victim == Owner
		|| !IsValid(Combat)
		|| !IsValid(OwnerStats)
		|| OwnerStats->IsDead()
		|| !IsValid(Ability)
		|| !IsValid(TargetEffectDefinition.Get())
		|| HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| HitData.Attacker.Get() != Owner
		|| Ability->GetOwnerCharacter() != Owner
		|| !Combat->IsAttackFromCurrentPassiveActivation(HitData)
		|| !MVWeaponPassiveConditions::MatchesAttackFilter(AttackFilter, *Combat, Ability)
		|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(*this, *Owner))
	{
		return;
	}

	PruneInactiveTargets(*Owner);
	UMVStatComponent* VictimStats = Victim->FindComponentByClass<UMVStatComponent>();
	UMVStatusEffectComponent* VictimEffects = Victim->FindComponentByClass<UMVStatusEffectComponent>();
	if (!IsValid(VictimStats) || VictimStats->IsDead() || VictimStats->CurrentHP <= 0.0f || !IsValid(VictimEffects))
	{
		return;
	}

	FMVStatusEffectSpec Spec;
	Spec.Definition = TargetEffectDefinition;
	Spec.SourceActor = Owner;
	Spec.StackDelta = FMath::Max(1, StackDelta);

	const TWeakObjectPtr<UMVStatusEffectComponent> TargetKey(VictimEffects);
	const FMVStatusEffectHandle AppliedHandle = VictimEffects->ApplyStatusEffect(Spec);
	VictimEffects = TargetKey.Get();
	if (!IsValid(VictimEffects))
	{
		TrackedEffects.Remove(TargetKey);
		return;
	}

	const bool bAppliedEffectActive = AppliedHandle.IsValid()
		&& VictimEffects->GetActiveEffects().ContainsByPredicate(
			[AppliedHandle](const FMVStatusEffectInstance& Candidate)
			{
				return Candidate.Handle == AppliedHandle;
			});
	if (!bAppliedEffectActive)
	{
		const FMVStatusEffectHandle* TrackedHandle = TrackedEffects.Find(TargetKey);
		if (TrackedHandle && *TrackedHandle == AppliedHandle)
		{
			TrackedEffects.Remove(TargetKey);
			UnbindTargetDeath(VictimEffects);
		}
		return;
	}

	// 효과 적용 중 대상이 죽거나 본체가 제거될 수 있다.
	if (!IsValid(VictimStats) || VictimStats->IsDead() || VictimStats->CurrentHP <= 0.0f
		|| !IsValid(OwnerStats) || OwnerStats->IsDead()
		|| !IsValid(Owner) || BoundOwner.Get() != Owner
		|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(*this, *Owner))
	{
		VictimEffects->RemoveStatusEffect(AppliedHandle, EMVStatusEffectRemovalReason::Manual);
		return;
	}

	TrackedEffects.Add(TargetKey, AppliedHandle);
	VictimStats->OnDeathStarted.AddUniqueDynamic(this, &UMVApplyEffectToHitTargetBehavior::HandleTargetDeathStarted);
}

void UMVApplyEffectToHitTargetBehavior::HandleOwnerDeathStarted(const FMVDeathContext& DeathContext)
{
	if (DeathContext.DeadActor.Get() == BoundOwner.Get())
	{
		RemoveTrackedEffects();
	}
}

void UMVApplyEffectToHitTargetBehavior::HandleTargetDeathStarted(const FMVDeathContext& DeathContext)
{
	const AActor* Target = DeathContext.DeadActor.Get();
	UMVStatusEffectComponent* TargetEffects = IsValid(Target)
		? Target->FindComponentByClass<UMVStatusEffectComponent>()
		: nullptr;
	if (!IsValid(TargetEffects))
	{
		return;
	}

	FMVStatusEffectHandle Handle;
	if (TrackedEffects.RemoveAndCopyValue(TWeakObjectPtr<UMVStatusEffectComponent>(TargetEffects), Handle))
	{
		UnbindTargetDeath(TargetEffects);
		TargetEffects->RemoveStatusEffect(Handle, EMVStatusEffectRemovalReason::Manual);
	}
}

void UMVApplyEffectToHitTargetBehavior::UnbindTargetDeath(UMVStatusEffectComponent* TargetEffects)
{
	const AActor* Target = IsValid(TargetEffects) ? TargetEffects->GetOwner() : nullptr;
	if (UMVStatComponent* Stats = IsValid(Target) ? Target->FindComponentByClass<UMVStatComponent>() : nullptr)
	{
		Stats->OnDeathStarted.RemoveDynamic(this, &UMVApplyEffectToHitTargetBehavior::HandleTargetDeathStarted);
	}
}

void UMVApplyEffectToHitTargetBehavior::PruneInactiveTargets(const AMVCharacterBase& Owner)
{
	for (auto It = TrackedEffects.CreateIterator(); It; ++It)
	{
		UMVStatusEffectComponent* TargetEffects = It.Key().Get();
		if (!IsValid(TargetEffects)
			|| TargetEffects->FindStatusEffectHandle(TargetEffectDefinition.Get(), &Owner) != It.Value())
		{
			UnbindTargetDeath(TargetEffects);
			It.RemoveCurrent();
		}
	}
}

void UMVApplyEffectToHitTargetBehavior::RemoveTrackedEffects()
{
	// 효과 제거 알림이 다시 이 행동의 정리를 호출할 수 있으므로 목록을 먼저 비운다.
	const auto EffectsToRemove = MoveTemp(TrackedEffects);
	for (const auto& Entry : EffectsToRemove)
	{
		if (UMVStatusEffectComponent* TargetEffects = Entry.Key.Get())
		{
			UnbindTargetDeath(TargetEffects);
			TargetEffects->RemoveStatusEffect(Entry.Value, EMVStatusEffectRemovalReason::Manual);
		}
	}
}
