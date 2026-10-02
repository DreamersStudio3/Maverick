#include "Weapon/Passive/GreatSword/MVWeightOfRuinPassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"

void UMVWeightOfRuinPassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
    if (Instance.Handle != GetEffectHandle())
    {
        return;
    }

    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
    UMVCombatComponent* Combat = IsValid(Owner) ? Owner->CombatComponent.Get() : nullptr;
    UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;

    if (!IsValid(Combat)
        || !IsValid(Stats)
        || Instance.SourceActor.Get() != Owner)
    {
        return;
    }

    const FGuid NewDodgeCostHandle =
        Stats->AddDodgeStaminaCostModifier(DodgeStaminaCostIncreaseRatio);
    if (!NewDodgeCostHandle.IsValid())
    {
        return;
    }

    BoundStatComponent = Stats;
    DodgeStaminaCostModifierHandle = NewDodgeCostHandle;

    BoundCombatComponent = Combat;
    OutgoingDamageDelegateHandle =
        Combat->OnModifyOutgoingAttackDamage.AddUObject(
            this,
            &UMVWeightOfRuinPassiveBehavior::HandleOutgoingAttackDamage);
}

void UMVWeightOfRuinPassiveBehavior::OnRemoved_Implementation(
    const FMVStatusEffectInstance& /*Instance*/,
    EMVStatusEffectRemovalReason /*RemovalReason*/)
{
    if (UMVCombatComponent* Combat = BoundCombatComponent.Get())
    {
        if (OutgoingDamageDelegateHandle.IsValid())
        {
            Combat->OnModifyOutgoingAttackDamage.Remove(
                OutgoingDamageDelegateHandle);
        }
    }

    if (UMVStatComponent* Stats = BoundStatComponent.Get())
    {
        if (DodgeStaminaCostModifierHandle.IsValid())
        {
            Stats->RemoveDodgeStaminaCostModifier(
                DodgeStaminaCostModifierHandle);
        }
    }

    OutgoingDamageDelegateHandle = FDelegateHandle();
    BoundCombatComponent.Reset();
    DodgeStaminaCostModifierHandle = FGuid();
    BoundStatComponent.Reset();
}

void UMVWeightOfRuinPassiveBehavior::HandleOutgoingAttackDamage(
    const FMVResolvedHitData& HitData,
    float& InOutDamage)
{
    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;

    if (!IsValid(Owner)
        || HitData.Origin != EMVResolvedHitOrigin::AttackCollision
        || HitData.Attacker.Get() != Owner
        || !IsValid(HitData.Victim.Get()))
    {
        return;
    }

    UMVStatComponent* Stats = Owner->StatComponent.Get();
    if (!IsValid(Stats) || Stats->IsDead())
    {
        return;
    }

    const FMVStatusEffectHandle LocalEffectHandle = GetEffectHandle();
    const FMVStatusEffectInstance* ActiveInstance =
        Effects->GetActiveEffects().FindByPredicate(
            [LocalEffectHandle](const FMVStatusEffectInstance& Instance)
            {
                return Instance.Handle == LocalEffectHandle;
            });

    if (!ActiveInstance
        || !ActiveInstance->IsValid()
        || ActiveInstance->SourceActor.Get() != Owner)
    {
        return;
    }

    const float BoostedDamage = InOutDamage * (1.0f + FMath::Max(0.0f, BonusDamageRatio));

    if (!FMath::IsFinite(BoostedDamage))
    {
        return;
    }

    InOutDamage = BoostedDamage;
}