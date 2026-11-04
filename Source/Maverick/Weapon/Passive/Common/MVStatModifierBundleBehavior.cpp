#include "Weapon/Passive/Common/MVStatModifierBundleBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Struct/MVHitTypes.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"

namespace
{
	FGuid RegisterPassiveBundleModifier(UMVStatComponent& Stats, const FMVPassiveStatModifier& Modifier)
	{
		switch (Modifier.Kind)
		{
		case EMVPassiveStatModifierKind::Defence:
			return Stats.AddDefenceModifier(Modifier.BonusRatio);
		case EMVPassiveStatModifierKind::MaxHP:
			return Stats.AddMaxHPModifier(Modifier.BonusRatio);
		case EMVPassiveStatModifierKind::Poise:
			return Stats.AddPoiseModifier(Modifier.BonusRatio);
		case EMVPassiveStatModifierKind::DodgeStaminaCost:
			return Stats.AddDodgeStaminaCostModifier(Modifier.BonusRatio);
		}

		return FGuid();
	}

	void UnregisterPassiveBundleModifier(
		UMVStatComponent& Stats, EMVPassiveStatModifierKind Kind, FGuid Handle)
	{
		switch (Kind)
		{
		case EMVPassiveStatModifierKind::Defence:
			Stats.RemoveDefenceModifier(Handle);
			break;
		case EMVPassiveStatModifierKind::MaxHP:
			Stats.RemoveMaxHPModifier(Handle);
			break;
		case EMVPassiveStatModifierKind::Poise:
			Stats.RemovePoiseModifier(Handle);
			break;
		case EMVPassiveStatModifierKind::DodgeStaminaCost:
			Stats.RemoveDodgeStaminaCostModifier(Handle);
			break;
		}
	}
}

bool FMVRegisteredPassiveStatModifiers::TryApply(
	const UMVStatusEffectBehavior& Behavior,
	const FMVStatusEffectInstance& Instance,
	TArrayView<const FMVPassiveStatModifier> Modifiers)
{
	if (bApplying || !Registered.IsEmpty() || Modifiers.IsEmpty() || Instance.Handle != Behavior.GetEffectHandle())
	{
		return false;
	}

	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(Behavior);
	UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;
	if (!IsValid(Stats)
		|| Instance.SourceActor.Get() != Owner
		|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(Behavior, *Owner))
	{
		return false;
	}

	const TGuardValue<bool> ApplyingGuard(bApplying, true);
	const TWeakObjectPtr<UMVStatComponent> ApplyingStats = Stats;
	const TWeakObjectPtr<AMVCharacterBase> ApplyingOwner = Owner;
	BoundStats = Stats;
	for (const FMVPassiveStatModifier& Modifier : Modifiers)
	{
		const EMVPassiveStatModifierKind Kind = Modifier.Kind;
		const FGuid Handle = RegisterPassiveBundleModifier(*Stats, Modifier);
		if (!Handle.IsValid())
		{
			Reset();
			return false;
		}

		// 최대 체력 변경 알림 중 효과가 제거되면 이전 핸들은 OnRemoved에서 정리된다.
		// 이번 핸들은 아직 기록되지 않았으므로 여기서 해제한다.
		if (!ApplyingStats.IsValid()
			|| !ApplyingOwner.IsValid()
			|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(Behavior, *ApplyingOwner.Get()))
		{
			if (UMVStatComponent* RemainingStats = ApplyingStats.Get())
			{
				UnregisterPassiveBundleModifier(*RemainingStats, Kind, Handle);
			}
			Reset();
			return false;
		}

		Registered.Add({Kind, Handle});
	}

	return true;
}

void FMVRegisteredPassiveStatModifiers::Reset()
{
	TArray<FRegisteredModifier> ToRemove = MoveTemp(Registered);
	const TWeakObjectPtr<UMVStatComponent> RemovedFrom = BoundStats;
	BoundStats.Reset();

	for (int32 Index = ToRemove.Num() - 1; Index >= 0; --Index)
	{
		if (UMVStatComponent* Stats = RemovedFrom.Get())
		{
			const FRegisteredModifier& Modifier = ToRemove[Index];
			UnregisterPassiveBundleModifier(*Stats, Modifier.Kind, Modifier.Handle);
		}
	}
}

void UMVStatModifierBundleBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	UMVCombatComponent* Combat = IsValid(Owner) ? Owner->CombatComponent.Get() : nullptr;
	if (DamageMode == EMVPassiveBundleDamageMode::OutgoingAttack && !IsValid(Combat))
	{
		return;
	}

	if (!RegisteredModifiers.TryApply(*this, Instance, Modifiers))
	{
		return;
	}

	switch (DamageMode)
	{
	case EMVPassiveBundleDamageMode::Incoming:
		if (UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr; IsValid(Stats))
		{
			IncomingDamageSource = Stats;
			DamageDelegateHandle = Stats->OnModifyIncomingDamage.AddUObject(
				this, &UMVStatModifierBundleBehavior::HandleIncomingDamage);
			return;
		}
		break;
	case EMVPassiveBundleDamageMode::OutgoingAttack:
		if (IsValid(Combat))
		{
			OutgoingDamageSource = Combat;
			DamageDelegateHandle = Combat->OnModifyOutgoingAttackDamage.AddUObject(
				this, &UMVStatModifierBundleBehavior::HandleOutgoingAttackDamage);
			return;
		}
		break;
	case EMVPassiveBundleDamageMode::None:
		return;
	}

	RegisteredModifiers.Reset();
}

void UMVStatModifierBundleBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/, EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVStatComponent* Stats = IncomingDamageSource.Get())
	{
		Stats->OnModifyIncomingDamage.Remove(DamageDelegateHandle);
	}
	if (UMVCombatComponent* Combat = OutgoingDamageSource.Get())
	{
		Combat->OnModifyOutgoingAttackDamage.Remove(DamageDelegateHandle);
	}

	DamageDelegateHandle.Reset();
	IncomingDamageSource.Reset();
	OutgoingDamageSource.Reset();
	RegisteredModifiers.Reset();
}

void UMVStatModifierBundleBehavior::HandleIncomingDamage(const FMVResolvedHitData& HitData, float& InOutDamage)
{
	const AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	if (MVWeaponPassiveConditions::CanModifyIncomingDamage(*this, IncomingDamageSource.Get(), Owner, HitData, InOutDamage))
	{
		MVWeaponPassiveConditions::ApplyFiniteDamageMultiplier(DamageMultiplier, InOutDamage);
	}
}

void UMVStatModifierBundleBehavior::HandleOutgoingAttackDamage(const FMVResolvedHitData& HitData, float& InOutDamage)
{
	const UMVCombatComponent* Combat = OutgoingDamageSource.Get();
	const AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	if (!IsValid(Combat)
		|| !IsValid(Owner)
		|| HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| HitData.Attacker.Get() != Owner
		|| !IsValid(HitData.Victim.Get())
		|| !Combat->IsAttackFromCurrentPassiveActivation(HitData))
	{
		return;
	}

	const UMVStatComponent* Stats = Owner->StatComponent.Get();
	if (!IsValid(Stats)
		|| Stats->IsDead()
		|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(*this, *Owner))
	{
		return;
	}

	MVWeaponPassiveConditions::ApplyFiniteDamageMultiplier(DamageMultiplier, InOutDamage);
}
