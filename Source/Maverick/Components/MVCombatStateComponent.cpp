#include "Components/MVCombatStateComponent.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "Engine/World.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "Tags/MVGameplayTags.h"

UMVCombatStateComponent::UMVCombatStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UMVCombatStateComponent::BeginPlay()
{
	Super::BeginPlay();

	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (OwnerCharacter->CombatComponent)
	{
		OwnerCharacter->CombatComponent->OnCombatActionStarted.RemoveDynamic(this, &UMVCombatStateComponent::HandleCombatActionStarted);
		OwnerCharacter->CombatComponent->OnCombatActionStarted.AddUniqueDynamic(this, &UMVCombatStateComponent::HandleCombatActionStarted);
	}

	if (UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(this))
	{
		HitResolver->OnHitResolved.RemoveDynamic(this, &UMVCombatStateComponent::HandleHitResolved);
		HitResolver->OnHitResolved.AddUniqueDynamic(this, &UMVCombatStateComponent::HandleHitResolved);
	}
}

void UMVCombatStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		if (OwnerCharacter->CombatComponent)
		{
			OwnerCharacter->CombatComponent->OnCombatActionStarted.RemoveDynamic(this, &UMVCombatStateComponent::HandleCombatActionStarted);
		}
	}

	if (UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(this))
	{
		HitResolver->OnHitResolved.RemoveDynamic(this, &UMVCombatStateComponent::HandleHitResolved);
	}

	for (const TWeakObjectPtr<AActor>& ThreatActor : ActiveAggroThreats)
	{
		AActor* Threat = ThreatActor.Get();
		if (!Threat)
		{
			continue;
		}

		if (UMVCombatComponent* ThreatCombatComponent = Threat->FindComponentByClass<UMVCombatComponent>())
		{
			ThreatCombatComponent->OnCombatActionStarted.RemoveDynamic(this, &UMVCombatStateComponent::HandleCombatActionStarted);
		}
	}

	ActiveAggroThreats.Reset();

	Super::EndPlay(EndPlayReason);
}

void UMVCombatStateComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInCombat)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const double TimeSinceLastActivity = World->GetTimeSeconds() - LastCombatActivityTime;

	if (TimeSinceLastActivity < CombatExitDelay || HasCombatExitBlocker())
	{
		return;
	}

	SetInCombat(false);
}

bool UMVCombatStateComponent::IsOutOfCombat() const
{
	return !bInCombat && !HasCombatExitBlocker();
}

void UMVCombatStateComponent::NotifyCombatActivity()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	LastCombatActivityTime = World->GetTimeSeconds();
	SetInCombat(true);
}

void UMVCombatStateComponent::SetAggroThreatActive(AActor* ThreatActor, bool bActive)
{
	if (!IsValid(ThreatActor) || ThreatActor == GetOwner())
	{
		return;
	}

	const TWeakObjectPtr<AActor> ThreatKey(ThreatActor);
	UMVCombatComponent* ThreatCombatComponent = ThreatActor->FindComponentByClass<UMVCombatComponent>();

	if (bActive)
	{
		if (ActiveAggroThreats.Contains(ThreatKey))
		{
			return;
		}

		ActiveAggroThreats.Add(ThreatKey);

		if (ThreatCombatComponent)
		{
			ThreatCombatComponent->OnCombatActionStarted.AddUniqueDynamic(this, &UMVCombatStateComponent::HandleCombatActionStarted);
		}

		NotifyCombatActivity();
		return;
	}

	ActiveAggroThreats.Remove(ThreatKey);

	if (ThreatCombatComponent)
	{
		ThreatCombatComponent->OnCombatActionStarted.RemoveDynamic(this, &UMVCombatStateComponent::HandleCombatActionStarted);
	}
}

void UMVCombatStateComponent::HandleCombatActionStarted(const FMVCombatActionEvent& Event)
{
	AActor* Instigator = Event.Instigator.Get();
	if (!Instigator)
	{
		return;
	}

	const TWeakObjectPtr<AActor> InstigatorKey(Instigator);

	if (Instigator == GetOwner() || ActiveAggroThreats.Contains(InstigatorKey))
	{
		NotifyCombatActivity();
	}
}

void UMVCombatStateComponent::HandleHitResolved(const FMVResolvedHitData& HitData)
{
	const AActor* OwnerActor = GetOwner();
	if (HitData.Attacker.Get() == OwnerActor || HitData.Victim.Get() == OwnerActor)
	{
		NotifyCombatActivity();
	}
}

bool UMVCombatStateComponent::HasCombatExitBlocker() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return true;
	}

	if (HasActiveAggroThreat())
	{
		return true;
	}

	if (OwnerCharacter->GetCharacterIsLying())
	{
		return true;
	}

	const UMVStatComponent* StatComponent = OwnerCharacter->StatComponent;
	if (StatComponent && (StatComponent->IsGroggy() || StatComponent->IsStaminaExhausted()))
	{
		return true;
	}

	return HasActiveDebuff();
}

bool UMVCombatStateComponent::HasActiveDebuff() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVStatusEffectComponent* StatusEffectComponent =
		OwnerCharacter ? OwnerCharacter->StatusEffectComponent : nullptr;

	if (!StatusEffectComponent)
	{
		return false;
	}

	for (const FMVStatusEffectInstance& Instance : StatusEffectComponent->GetActiveEffects())
	{
		const UMVStatusEffectDefinition* Definition = Instance.Definition.Get();

		if (Instance.IsValid()
			&& Definition
			&& Definition->EffectTags.HasTag(MVGameplayTags::StatusEffect_Type_Debuff))
		{
			return true;
		}
	}

	return false;
}

bool UMVCombatStateComponent::HasActiveAggroThreat() const
{
	for (const TWeakObjectPtr<AActor>& ThreatActor : ActiveAggroThreats)
	{
		if (ThreatActor.IsValid())
		{
			return true;
		}
	}

	return false;
}

void UMVCombatStateComponent::SetInCombat(const bool bNewInCombat)
{
	if (bInCombat == bNewInCombat)
	{
		return;
	}

	bInCombat = bNewInCombat;
	OnCombatStateChanged.Broadcast(bInCombat);
}
