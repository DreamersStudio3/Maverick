#include "StatusEffects/Behaviors/MVMoveSpeedPerStackBehavior.h"

#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "GameFramework/Actor.h"

void UMVMoveSpeedPerStackBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	ApplyStackBonus(Instance);
}

void UMVMoveSpeedPerStackBehavior::OnUpdated_Implementation(
	const FMVStatusEffectInstance& Instance,
	int32 /*PreviousStacks*/)
{
	ApplyStackBonus(Instance);
}

void UMVMoveSpeedPerStackBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	UMVStatComponent* Stats = ModifiedStatComponent.Get();
	if (IsValid(Stats) && ModifierHandle.IsValid())
	{
		Stats->RemoveMoveSpeedModifier(ModifierHandle);
	}

	ModifiedStatComponent.Reset();
	ModifierHandle = FGuid();
}

void UMVMoveSpeedPerStackBehavior::ApplyStackBonus(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AActor* Owner = IsValid(Effects) ? Effects->GetOwner() : nullptr;
	UMVStatComponent* Stats =
		IsValid(Owner) ? Owner->FindComponentByClass<UMVStatComponent>() : nullptr;
	if (!IsValid(Stats))
	{
		return;
	}

	const float BonusRatio =
		FMath::Max(0.0f, BonusRatioPerStack)
		* FMath::Max(0, Instance.CurrentStacks);

	if (!ModifierHandle.IsValid()
		|| !Stats->UpdateMoveSpeedModifier(ModifierHandle, BonusRatio))
	{
		ModifierHandle = Stats->AddMoveSpeedModifier(BonusRatio);
		ModifiedStatComponent = Stats;
	}
}