#include "StatusEffects/Behaviors/MVAttackSpeedPerStackBehavior.h"

#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "GameFramework/Actor.h"

void UMVAttackSpeedPerStackBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	ApplyStackBonus(Instance);
}

void UMVAttackSpeedPerStackBehavior::OnUpdated_Implementation(
	const FMVStatusEffectInstance& Instance,
	int32 /*PreviousStacks*/)
{
	ApplyStackBonus(Instance);
}

void UMVAttackSpeedPerStackBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (ModifiedStatComponent.IsValid() && ModifierHandle.IsValid())
	{
		ModifiedStatComponent->RemoveAttackSpeedModifier(ModifierHandle);
	}

	ModifiedStatComponent.Reset();
	ModifierHandle = FGuid();
}

void UMVAttackSpeedPerStackBehavior::ApplyStackBonus(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	UMVStatusEffectComponent* EffectComponent = GetOwningStatusEffectComponent();
	AActor* Owner = EffectComponent ? EffectComponent->GetOwner() : nullptr;
	UMVStatComponent* StatComponent = Owner ? Owner->FindComponentByClass<UMVStatComponent>() : nullptr;

	if (!IsValid(StatComponent))
	{
		return;
	}

	const float BonusRatio =
		FMath::Max(0.0f, BonusRatioPerStack)
		* FMath::Max(0, Instance.CurrentStacks);

	if (ModifierHandle.IsValid()
		&& StatComponent->UpdateAttackSpeedModifier(
			ModifierHandle, BonusRatio))
	{
		return;
	}

	ModifierHandle = StatComponent->AddAttackSpeedModifier(BonusRatio);
	ModifiedStatComponent = StatComponent;
}
