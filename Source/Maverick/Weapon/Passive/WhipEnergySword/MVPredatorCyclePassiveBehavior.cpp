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

	Combat->OnValidatedAttackHitAfterDamage.AddUniqueDynamic(
		this,
		&UMVPredatorCyclePassiveBehavior::HandleValidatedAttackHitAfterDamage);
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

	BoundOwner.Reset();
	BoundCombatComponent.Reset();
	BoundStatComponent.Reset();
	PassiveDefinition.Reset();
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
		|| !Combat->IsAttackFromCurrentPassiveActivation(HitData)
		|| !Ability->bAbilityCostConsumed
		|| Ability->GetAttackInstanceId() != HitData.AttackInstanceId
		|| HitData.AttackInstanceId == INDEX_NONE
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

	// Combat가 원래 스킬 사용 회차와 공격별 중복을 승인한 경우에만 MP를 돌려준다.
	if (Combat->RefundSkillMainCooldownForExecution(SkillIndex, Ability))
	{
		Stats->RecoverMP(RefundAmount);
	}
}
