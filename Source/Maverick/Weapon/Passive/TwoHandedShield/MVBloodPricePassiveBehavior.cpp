#include "Weapon/Passive/TwoHandedShield/MVBloodPricePassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"

void UMVBloodPricePassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects)
		? Cast<AMVCharacterBase>(Effects->GetOwner())
		: nullptr;
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
		this, &UMVBloodPricePassiveBehavior::HandleDamageApplied);
	Stats->OnDeathStarted.AddUniqueDynamic(
		this, &UMVBloodPricePassiveBehavior::HandleOwnerDeathStarted);
}

void UMVBloodPricePassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		Stats->OnDamageApplied.RemoveDynamic(
			this, &UMVBloodPricePassiveBehavior::HandleDamageApplied);
		Stats->OnDeathStarted.RemoveDynamic(
			this, &UMVBloodPricePassiveBehavior::HandleOwnerDeathStarted);
	}

	BoundStatComponent.Reset();
	AccumulatedDamageRatio = 0.0;
}

void UMVBloodPricePassiveBehavior::HandleDamageApplied(
	float AppliedDamage,
	float /*PreviousHP*/,
	float /*CurrentHP*/,
	const FMVResolvedHitData& HitData)
{
	UMVStatComponent* Stats = BoundStatComponent.Get();
	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects)
		? Cast<AMVCharacterBase>(Effects->GetOwner())
		: nullptr;

	if (!IsValid(Stats)
		|| !IsValid(Owner)
		|| Stats->GetOwner() != Owner
		|| Stats->IsDead()
		|| HitData.Victim.Get() != Owner
		|| !FMath::IsFinite(AppliedDamage)
		|| AppliedDamage <= 0.0f
		|| !FMath::IsFinite(Stats->MaxHP)
		|| Stats->MaxHP <= 0.0f)
	{
		return;
	}

	const double Threshold = static_cast<double>(
		FMath::Clamp(DamageThresholdRatio, 0.001f, 1.0f));
	const double DamageRatio =
		static_cast<double>(AppliedDamage) / static_cast<double>(Stats->MaxHP);

	AccumulatedDamageRatio += DamageRatio;

	// float로 설정한 0.05와 계산 결과의 미세한 차이로
	// 정확히 5%에 도달했을 때 발동을 놓치지 않도록 한다.
	const int32 TriggerCount =
		FMath::FloorToInt((AccumulatedDamageRatio + 1e-8) / Threshold);

	if (TriggerCount > 0)
	{
		AccumulatedDamageRatio = FMath::Max(
			0.0, AccumulatedDamageRatio - Threshold * TriggerCount);

		if (UMVCombatComponent* Combat = Owner->CombatComponent.Get())
		{
			Combat->ReduceOngoingSkillMainCooldowns(
				CooldownRefundRatio,
				TriggerCount);
		}
	}
}

void UMVBloodPricePassiveBehavior::HandleOwnerDeathStarted(const FMVDeathContext& DeathContext)
{
	UMVStatComponent* Stats = BoundStatComponent.Get();
	if (!IsValid(Stats)
		|| DeathContext.DeadActor.Get() != Stats->GetOwner())
	{
		return;
	}

	AccumulatedDamageRatio = 0.0;
}
