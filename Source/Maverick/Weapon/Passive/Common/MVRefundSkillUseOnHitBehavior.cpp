#include "Weapon/Passive/Common/MVRefundSkillUseOnHitBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Weapon/Passive/Common/MVWeaponPassiveConditions.h"

void UMVRefundSkillUseOnHitBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	AMVCharacterBase* Owner = MVWeaponPassiveConditions::ResolveOwner(*this);
	UMVCombatComponent* Combat = IsValid(Owner) ? Owner->CombatComponent.Get() : nullptr;
	UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;

	if (!IsValid(Combat)
		|| !IsValid(Stats)
		|| Stats->IsDead()
		|| Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	BoundOwner = Owner;
	BoundCombatComponent = Combat;
	BoundStatComponent = Stats;

	Combat->OnValidatedAttackHitAfterDamage.AddUniqueDynamic(
		this,
		&UMVRefundSkillUseOnHitBehavior::HandleValidatedAttackHitAfterDamage);
}

void UMVRefundSkillUseOnHitBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVCombatComponent* Combat = BoundCombatComponent.Get())
	{
		Combat->OnValidatedAttackHitAfterDamage.RemoveDynamic(
			this,
			&UMVRefundSkillUseOnHitBehavior::HandleValidatedAttackHitAfterDamage);
	}

	BoundOwner.Reset();
	BoundCombatComponent.Reset();
	BoundStatComponent.Reset();
}

void UMVRefundSkillUseOnHitBehavior::HandleValidatedAttackHitAfterDamage(
	const FMVResolvedHitData& HitData,
	UMVAbilityBase* Ability)
{
	AMVCharacterBase* Owner = BoundOwner.Get();
	UMVCombatComponent* Combat = BoundCombatComponent.Get();
	UMVStatComponent* Stats = BoundStatComponent.Get();
	AMVEnemy* Enemy = Cast<AMVEnemy>(HitData.Victim.Get());

	if (!IsValid(Owner)
		|| !IsValid(Combat)
		|| !IsValid(Stats)
		|| Stats->IsDead()
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
		|| !MVWeaponPassiveConditions::HasActiveSelfAppliedEffect(*this, *Owner))
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

	const float Chance = FMath::IsFinite(ActivationChance)
		? FMath::Clamp(ActivationChance, 0.0f, 1.0f)
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
