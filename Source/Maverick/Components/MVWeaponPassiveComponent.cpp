#include "Components/MVWeaponPassiveComponent.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatusEffectComponent.h"
#include "Components/MVWeaponComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "Weapon/Passive/MVWeaponPassiveSet.h"

UMVWeaponPassiveComponent::UMVWeaponPassiveComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMVWeaponPassiveComponent::BeginPlay()
{
    Super::BeginPlay();

    AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
    UMVWeaponComponent* WeaponComponent = OwnerCharacter ? OwnerCharacter->WeaponComponent.Get() : nullptr;
    UMVStatusEffectComponent* EffectComponent = OwnerCharacter ? OwnerCharacter->StatusEffectComponent.Get() : nullptr;

    if (!IsValid(WeaponComponent) || !IsValid(EffectComponent))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Weapon passive component requires weapon and status effect components. Owner=%s"),
            *GetNameSafe(GetOwner()));
        return;
    }

    BoundWeaponComponent = WeaponComponent;
    BoundStatusEffectComponent = EffectComponent;

    WeaponComponent->OnEquippedWeaponChanged.AddUniqueDynamic(
        this, &UMVWeaponPassiveComponent::HandleEquippedWeaponChanged);

    HandleEquippedWeaponChanged(WeaponComponent->GetEquippedWeaponState());
}

void UMVWeaponPassiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (BoundWeaponComponent.IsValid())
    {
        BoundWeaponComponent->OnEquippedWeaponChanged.RemoveDynamic(
            this, &UMVWeaponPassiveComponent::HandleEquippedWeaponChanged);
    }

    ClearActivePassives();
    BoundWeaponComponent.Reset();
    BoundStatusEffectComponent.Reset();

    Super::EndPlay(EndPlayReason);
}

void UMVWeaponPassiveComponent::HandleEquippedWeaponChanged(const FMVEquippedWeaponState& WeaponState)
{
    ClearActivePassives();

    if (!WeaponState.bValid || WeaponState.PassiveSet.IsNull()
        || !BoundStatusEffectComponent.IsValid())
    {
        return;
    }

    UMVWeaponPassiveSet* PassiveSet = WeaponState.PassiveSet.LoadSynchronous();
    if (!IsValid(PassiveSet))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Failed to load weapon passive set. Owner=%s Path=%s"),
            *GetNameSafe(GetOwner()),
            *WeaponState.PassiveSet.ToSoftObjectPath().ToString());
        return;
    }

    UMVStatusEffectComponent* EffectComponent = BoundStatusEffectComponent.Get();
    TSet<const UMVStatusEffectDefinition*> SeenDefinitions;

    for (const TObjectPtr<UMVStatusEffectDefinition>& DefinitionRef : PassiveSet->PassiveDefinitions)
    {
        UMVStatusEffectDefinition* Definition = DefinitionRef.Get();
        if (!IsValid(Definition) || SeenDefinitions.Contains(Definition))
        {
            continue;
        }
        SeenDefinitions.Add(Definition);

        const bool bSupportedStackPolicy =
            Definition->StackPolicy == EMVStatusEffectStackPolicy::NoStack
            || Definition->StackPolicy == EMVStatusEffectStackPolicy::AddStack;

        if (Definition->DurationPolicy != EMVStatusEffectDurationPolicy::Infinite
            || !bSupportedStackPolicy
            || Definition->InstanceScope != EMVStatusEffectInstanceScope::OnePerSource)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Weapon passive definition must use Infinite, NoStack or AddStack, and OnePerSource. Owner=%s Definition=%s"),
                *GetNameSafe(GetOwner()),
                *GetNameSafe(Definition));
            continue;
        }

        // 이미 다른 경로에서 켠 효과를 이 컴포넌트의 것으로 착각해
        // 무기 교체 때 제거하지 않도록 한다.
        if (EffectComponent->HasStatusEffect(Definition, GetOwner()))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Weapon passive is already active and will not be owned by this component. Owner=%s Definition=%s"),
                *GetNameSafe(GetOwner()),
                *GetNameSafe(Definition));
            continue;
        }

        FMVStatusEffectSpec Spec;
        Spec.Definition = Definition;
        Spec.SourceActor = GetOwner();
        Spec.StackDelta = 1;

        const FMVStatusEffectHandle Handle = EffectComponent->ApplyStatusEffect(Spec);
        if (Handle.IsValid())
        {
            ActivePassiveHandles.Add(Handle);
        }
    }
}

void UMVWeaponPassiveComponent::ClearActivePassives()
{
    UMVStatusEffectComponent* EffectComponent = BoundStatusEffectComponent.Get();

    for (const FMVStatusEffectHandle Handle : ActivePassiveHandles)
    {
        if (IsValid(EffectComponent))
        {
            EffectComponent->RemoveStatusEffect(
                Handle, EMVStatusEffectRemovalReason::Manual);
        }
    }

    ActivePassiveHandles.Reset();
}
