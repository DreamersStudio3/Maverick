// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Passive/Katana/MVStormBladePassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "Components/MVStatComponent.h"

void UMVStormBladePassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
    if (Instance.Handle != GetEffectHandle()
        || !IsValid(SpeedEffectDefinition.Get()))
    {
        return;
    }

    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AMVCharacterBase* Owner = Effects ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
    UMVCombatComponent* Combat = Owner ? Owner->CombatComponent.Get() : nullptr;
    UMVStatComponent* Stats = Owner ? Owner->StatComponent.Get() : nullptr;

    if (!IsValid(Owner) || !IsValid(Combat) || !IsValid(Stats))
    {
        return;
    }

    BoundCharacter = Owner;
    BoundCombatComponent = Combat;
    BoundStatComponent = Stats;
    PassiveDefinition = Instance.Definition.Get();

    Combat->OnValidatedAttackHit.AddUniqueDynamic(
        this,
        &UMVStormBladePassiveBehavior::HandleValidatedAttackHit);
    Combat->OnAttackMissed.AddUniqueDynamic(
        this,
        &UMVStormBladePassiveBehavior::HandleAttackMissed);
    Owner->OnDamaged.AddUniqueDynamic(
        this,
        &UMVStormBladePassiveBehavior::HandleOwnerDamaged);
    Stats->OnDeathStarted.AddUniqueDynamic(
        this,
        &UMVStormBladePassiveBehavior::HandleOwnerDeathStarted);
}

void UMVStormBladePassiveBehavior::OnRemoved_Implementation(
    const FMVStatusEffectInstance& /*Instance*/,
    EMVStatusEffectRemovalReason /*RemovalReason*/)
{
    if (BoundCombatComponent.IsValid())
    {
        BoundCombatComponent->OnValidatedAttackHit.RemoveDynamic(
            this,
            &UMVStormBladePassiveBehavior::HandleValidatedAttackHit);
        BoundCombatComponent->OnAttackMissed.RemoveDynamic(
            this,
            &UMVStormBladePassiveBehavior::HandleAttackMissed);
    }

    if (BoundCharacter.IsValid())
    {
        BoundCharacter->OnDamaged.RemoveDynamic(
            this,
            &UMVStormBladePassiveBehavior::HandleOwnerDamaged);
    }

    if (BoundStatComponent.IsValid())
    {
        BoundStatComponent->OnDeathStarted.RemoveDynamic(
            this,
            &UMVStormBladePassiveBehavior::HandleOwnerDeathStarted);
    }

    RemoveSpeedEffect(EMVStatusEffectRemovalReason::Manual);

    BoundCombatComponent.Reset();
    BoundStatComponent.Reset();
    BoundCharacter.Reset();
    PassiveDefinition.Reset();
}

void UMVStormBladePassiveBehavior::HandleValidatedAttackHit(
    const FMVResolvedHitData& HitData,
    UMVAbilityBase* Ability)
{
    AMVCharacterBase* Owner = BoundCharacter.Get();
    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();

    if (!IsValid(Owner) || !IsValid(Effects)
        || !IsValid(Ability)
        || !IsValid(SpeedEffectDefinition.Get())
        || !IsValid(PassiveDefinition.Get())
        || HitData.Origin != EMVResolvedHitOrigin::AttackCollision
        || HitData.Attacker.Get() != Owner
        || Ability->GetOwnerCharacter() != Owner
        || Effects->FindStatusEffectHandle(
            PassiveDefinition.Get(), Owner) != GetEffectHandle())
    {
        return;
    }

    UMVStatComponent* Stats = BoundStatComponent.Get();
    if (!IsValid(Stats) || Stats->IsDead())
    {
        return;
    }

    FMVStatusEffectSpec Spec;
    Spec.Definition = SpeedEffectDefinition;
    Spec.SourceActor = Owner;
    Spec.StackDelta = 1;

    Effects->ApplyStatusEffect(Spec);
}

void UMVStormBladePassiveBehavior::HandleAttackMissed(UMVAbilityBase* MissedAbility)
{
    if (IsValid(MissedAbility)
        && MissedAbility->GetOwnerCharacter() == BoundCharacter.Get())
    {
        RemoveSpeedEffect(EMVStatusEffectRemovalReason::Consumed);
    }
}

void UMVStormBladePassiveBehavior::HandleOwnerDamaged(const FMVResolvedHitData& HitData)
{
    if (HitData.Victim.Get() == BoundCharacter.Get())
    {
        RemoveSpeedEffect(EMVStatusEffectRemovalReason::Consumed);
    }
}

void UMVStormBladePassiveBehavior::HandleOwnerDeathStarted(
    const FMVDeathContext& DeathContext)
{
    if (DeathContext.DeadActor.Get() != BoundCharacter.Get())
    {
        return;
    }

    RemoveSpeedEffect(EMVStatusEffectRemovalReason::Consumed);
}

void UMVStormBladePassiveBehavior::RemoveSpeedEffect(EMVStatusEffectRemovalReason RemovalReason)
{
    UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
    AMVCharacterBase* Owner = BoundCharacter.Get();

    if (!IsValid(Effects) || !IsValid(Owner)
        || !IsValid(SpeedEffectDefinition.Get()))
    {
        return;
    }

    const FMVStatusEffectHandle SpeedHandle =
        Effects->FindStatusEffectHandle(
            SpeedEffectDefinition.Get(), Owner);
    if (!SpeedHandle.IsValid())
    {
        return;
    }

    Effects->RemoveStatusEffect(SpeedHandle, RemovalReason);
}
