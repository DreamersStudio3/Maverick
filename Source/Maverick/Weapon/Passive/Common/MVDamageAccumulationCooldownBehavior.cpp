#include "Weapon/Passive/Common/MVDamageAccumulationCooldownBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"

void UMVDamageAccumulationCooldownBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle() || BoundStatComponent.IsValid())
	{
		return;
	}

	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	UMVStatComponent* Stats = IsValid(Owner)
		? Owner->StatComponent.Get()
		: nullptr;

	if (!IsValid(Stats)
		|| Stats->IsDead()
		|| Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	AccumulatedDamageRatio = 0.0;
	BoundStatComponent = Stats;

	Stats->OnDamageApplied.AddUniqueDynamic(
		this, &UMVDamageAccumulationCooldownBehavior::HandleDamageApplied);
	Stats->OnDeathStarted.AddUniqueDynamic(
		this, &UMVDamageAccumulationCooldownBehavior::HandleOwnerDeathStarted);
}

void UMVDamageAccumulationCooldownBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		Stats->OnDamageApplied.RemoveDynamic(
			this, &UMVDamageAccumulationCooldownBehavior::HandleDamageApplied);
		Stats->OnDeathStarted.RemoveDynamic(
			this, &UMVDamageAccumulationCooldownBehavior::HandleOwnerDeathStarted);
	}

	BoundStatComponent.Reset();
	AccumulatedDamageRatio = 0.0;
}

void UMVDamageAccumulationCooldownBehavior::HandleDamageApplied(
	float AppliedDamage,
	float /*PreviousHP*/,
	float /*CurrentHP*/,
	const FMVResolvedHitData& HitData)
{
	UMVStatComponent* Stats = BoundStatComponent.Get();
	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);

	if (!IsValid(Stats)
		|| !IsValid(Owner)
		|| Stats->GetOwner() != Owner
		|| Stats->IsDead()
		|| HitData.Victim.Get() != Owner
		|| !FMath::IsFinite(AppliedDamage)
		|| AppliedDamage <= 0.0f
		|| !FMath::IsFinite(Stats->MaxHP)
		|| Stats->MaxHP <= 0.0f
		|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(*this, *Owner))
	{
		return;
	}

	const double Threshold = static_cast<double>(
		FMath::Clamp(AccumulatedDamageThresholdRatio, 0.001f, 1.0f));
	const double DamageRatio =
		static_cast<double>(AppliedDamage) / static_cast<double>(Stats->MaxHP);

	AccumulatedDamageRatio += DamageRatio;

	// 설정값과 누적값의 부동소수점 오차 때문에 경계에서 발동을 놓치지 않게 한다.
	const int32 TriggerCount =
		FMath::FloorToInt((AccumulatedDamageRatio + 1e-8) / Threshold);

	if (TriggerCount > 0)
	{
		AccumulatedDamageRatio = FMath::Max(
			0.0, AccumulatedDamageRatio - Threshold * TriggerCount);

		if (UMVCombatComponent* Combat = Owner->CombatComponent.Get())
		{
			Combat->ReduceOngoingSkillMainCooldowns(
				CooldownReductionRatio,
				TriggerCount);
		}
	}
}

void UMVDamageAccumulationCooldownBehavior::HandleOwnerDeathStarted(const FMVDeathContext& DeathContext)
{
	UMVStatComponent* Stats = BoundStatComponent.Get();
	if (!IsValid(Stats)
		|| DeathContext.DeadActor.Get() != Stats->GetOwner())
	{
		return;
	}

	AccumulatedDamageRatio = 0.0;
}
