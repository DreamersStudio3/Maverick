#include "Weapon/Passive/WhipEnergySword/MVPredatorCyclePassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "Components/MVWeaponComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"

void UMVPredatorCyclePassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle())
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
		|| Stats->IsDead()
		|| !IsValid(Instance.Definition.Get())
		|| Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundOwner = Owner;
	BoundCombatComponent = Combat;
	BoundStatComponent = Stats;
	PassiveDefinition = Instance.Definition.Get();
	RefundedAttackInstanceId = INDEX_NONE;

	Combat->OnValidatedAttackHitAfterDamage.AddUniqueDynamic(
		this,
		&UMVPredatorCyclePassiveBehavior::HandleValidatedAttackHitAfterDamage);
	Stats->OnDeathStarted.AddUniqueDynamic(
		this,
		&UMVPredatorCyclePassiveBehavior::HandleOwnerDeathStarted);
}

void UMVPredatorCyclePassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombatComponent.Get())
	{
		Combat->OnValidatedAttackHitAfterDamage.RemoveDynamic(
			this,
			&UMVPredatorCyclePassiveBehavior::HandleValidatedAttackHitAfterDamage);
	}

	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		Stats->OnDeathStarted.RemoveDynamic(
			this,
			&UMVPredatorCyclePassiveBehavior::HandleOwnerDeathStarted);
	}

	BoundOwner.Reset();
	BoundCombatComponent.Reset();
	BoundStatComponent.Reset();
	PassiveDefinition.Reset();
	RefundedAttackInstanceId = INDEX_NONE;
}

void UMVPredatorCyclePassiveBehavior::HandleValidatedAttackHitAfterDamage(
	const FMVResolvedHitData& HitData,
	UMVAbilityBase* Ability)
{
	AMVCharacterBase* Owner = BoundOwner.Get();
	UMVCombatComponent* Combat = BoundCombatComponent.Get();
	UMVStatComponent* Stats = BoundStatComponent.Get();
	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVEnemy* Enemy = Cast<AMVEnemy>(HitData.Victim.Get());

	if (!IsValid(Owner)
		|| !IsValid(Combat)
		|| !IsValid(Stats)
		|| Stats->IsDead()
		|| !IsValid(Effects)
		|| !IsValid(PassiveDefinition.Get())
		|| !IsValid(Ability)
		|| !IsValid(Enemy)
		|| !IsValid(Owner->WeaponComponent.Get())
		|| HitData.Origin != EMVResolvedHitOrigin::AttackCollision
		|| HitData.Attacker.Get() != Owner
		|| HitData.FinalDamage <= 0.0f
		|| Ability->GetOwnerCharacter() != Owner
		|| !Ability->bAbilityCostConsumed
		|| Ability->GetAttackInstanceId() != HitData.AttackInstanceId
		|| HitData.AttackInstanceId == INDEX_NONE
		|| HitData.AttackInstanceId == RefundedAttackInstanceId
		|| Effects->FindStatusEffectHandle(
			PassiveDefinition.Get(), Owner) != GetEffectHandle())
	{
		return;
	}

	const int32 SkillIndex = Ability->GetSourceSkillIndex();
	if (SkillIndex < 0
		|| SkillIndex >= MVCombatSkillSlots::Count
		|| (SkillIndex == MVCombatSkillSlots::R && Combat->bUseRSkillGauge))
	{
		return;
	}

	const FGameplayTag& SourceWeaponTag = Ability->GetSourceWeaponItemTag();
	if (!SourceWeaponTag.IsValid()
		|| HitData.WeaponSnapshot.ItemTag != SourceWeaponTag
		|| Owner->WeaponComponent->GetEquippedWeaponState().ItemTag != SourceWeaponTag)
	{
		return;
	}

	const float RefundAmount = Ability->GetConsumedMPThisExecution();
	if (!FMath::IsFinite(RefundAmount) || RefundAmount < 0.0f)
	{
		return;
	}

	const float Chance = FMath::IsFinite(RefundChance)
		? FMath::Clamp(RefundChance, 0.0f, 1.0f)
		: 0.0f;
	const float Roll = FMath::FRand();
	const bool bTriggered = Chance >= 1.0f
		|| (Chance > 0.0f && Roll < Chance);

	if (!bTriggered)
	{
		return;
	}

	// 회복이 다른 사건을 일으켜도 같은 실행의 두 번째 환원을 막는다.
	RefundedAttackInstanceId = HitData.AttackInstanceId;

	Combat->RefundSkillMainCooldownForExecution(SkillIndex, Ability);
	Stats->RecoverMP(RefundAmount);
}

void UMVPredatorCyclePassiveBehavior::HandleOwnerDeathStarted(const FMVDeathContext& DeathContext)
{
	if (DeathContext.DeadActor.Get() == BoundOwner.Get())
	{
		RefundedAttackInstanceId = INDEX_NONE;
	}
}
