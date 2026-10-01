#include "Weapon/Passive/GreatSword/MVMoonlightCondPassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"

void UMVMoonlightCondPassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
    if (Instance.Handle != GetEffectHandle())
    {
        return;
    }

    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AMVCharacterBase* Owner = Effects ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
    UMVCombatComponent* Combat = IsValid(Owner) ? Owner->CombatComponent.Get() : nullptr;
    UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;

    if (!IsValid(Combat) || !IsValid(Stats))
    {
        return;
    }

    BoundCombatComponent = Combat;
    BoundStatComponent = Stats;

    OutgoingDamageDelegateHandle =
        Combat->OnModifyOutgoingAttackDamage.AddUObject(
            this,
            &UMVMoonlightCondPassiveBehavior::HandleOutgoingAttackDamage);

    Combat->OnAttackMissed.AddUniqueDynamic(
        this,
        &UMVMoonlightCondPassiveBehavior::HandleAttackMissed);

    Stats->OnDeathStarted.AddUniqueDynamic(
        this,
        &UMVMoonlightCondPassiveBehavior::HandleOwnerDeathStarted);
}

void UMVMoonlightCondPassiveBehavior::OnRemoved_Implementation(
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

        Combat->OnAttackMissed.RemoveDynamic(
            this,
            &UMVMoonlightCondPassiveBehavior::HandleAttackMissed);
    }

    if (UMVStatComponent* Stats = BoundStatComponent.Get())
    {
        Stats->OnDeathStarted.RemoveDynamic(
            this,
            &UMVMoonlightCondPassiveBehavior::HandleOwnerDeathStarted);
    }

    OutgoingDamageDelegateHandle = FDelegateHandle();
    BoundCombatComponent.Reset();
    BoundStatComponent.Reset();
}

void UMVMoonlightCondPassiveBehavior::HandleOutgoingAttackDamage(
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

    if (!ActiveInstance || !ActiveInstance->IsValid()
        || ActiveInstance->SourceActor.Get() != Owner)
    {
        return;
    }

    const UMVStatusEffectDefinition* Definition = ActiveInstance->Definition.Get();
    if (!IsValid(Definition)
        || Definition->StackPolicy != EMVStatusEffectStackPolicy::AddStack
        || Definition->MaxStacks < 2)
    {
        return;
    }

    const int32 StacksBefore = ActiveInstance->CurrentStacks;

    if (StacksBefore >= Definition->MaxStacks)
    {
        const float BoostedDamage =
            InOutDamage * (1.0f + FMath::Max(0.0f, BonusDamageRatio));

        if (!FMath::IsFinite(BoostedDamage)
            || !Effects->SetStatusEffectStacks(LocalEffectHandle, 1))
        {
            return;
        }

        InOutDamage = BoostedDamage;
        return;
    }

    const int32 StacksAfter = StacksBefore + 1;
    Effects->SetStatusEffectStacks(LocalEffectHandle, StacksAfter);
}

void UMVMoonlightCondPassiveBehavior::HandleAttackMissed(
    UMVAbilityBase* MissedAbility)
{
    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;

    if (IsValid(MissedAbility)
        && IsValid(Owner)
        && MissedAbility->GetOwnerCharacter() == Owner)
    {
        ResetCharge();
    }
}

void UMVMoonlightCondPassiveBehavior::HandleOwnerDeathStarted(
    const FMVDeathContext& DeathContext)
{
    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AActor* Owner = IsValid(Effects) ? Effects->GetOwner() : nullptr;

    if (IsValid(Owner) && DeathContext.DeadActor.Get() == Owner)
    {
        ResetCharge();
    }
}

void UMVMoonlightCondPassiveBehavior::ResetCharge()
{
    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AActor* Owner = IsValid(Effects) ? Effects->GetOwner() : nullptr;

    if (!IsValid(Owner))
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

    if (!ActiveInstance || !ActiveInstance->IsValid()
        || ActiveInstance->SourceActor.Get() != Owner
        || ActiveInstance->CurrentStacks <= 1)
    {
        return;
    }

    Effects->SetStatusEffectStacks(LocalEffectHandle, 1);
}
