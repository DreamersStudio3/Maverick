#include "StatusEffects/Behaviors/MVStatModifierPerStackBehavior.h"

#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "GameFramework/Actor.h"

void UMVStatModifierPerStackBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	ApplyStackBonus(Instance);
}

void UMVStatModifierPerStackBehavior::OnUpdated_Implementation(
	const FMVStatusEffectInstance& Instance, int32 /*PreviousStacks*/)
{
	ApplyStackBonus(Instance);
}

void UMVStatModifierPerStackBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/, EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	RemoveModifier();
}

void UMVStatModifierPerStackBehavior::ApplyStackBonus(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	const UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AActor* Owner = IsValid(Effects) ? Effects->GetOwner() : nullptr;
	UMVStatComponent* Stats = IsValid(Owner) ? Owner->FindComponentByClass<UMVStatComponent>() : nullptr;
	if (ModifiedStatComponent.Get() != Stats)
	{
		RemoveModifier();
	}
	if (!IsValid(Stats))
	{
		return;
	}

	const float BonusRatio = FMath::Max(0.0f, BonusRatioPerStack) * FMath::Max(0, Instance.CurrentStacks);
	if (!FMath::IsFinite(BonusRatio))
	{
		return;
	}

	switch (Stat)
	{
	case EMVPerStackStat::AttackSpeed:
		if (!Stats->UpdateAttackSpeedModifier(ModifierHandle, BonusRatio))
		{
			ModifierHandle = Stats->AddAttackSpeedModifier(BonusRatio);
		}
		break;
	case EMVPerStackStat::MoveSpeed:
		if (!Stats->UpdateMoveSpeedModifier(ModifierHandle, BonusRatio))
		{
			ModifierHandle = Stats->AddMoveSpeedModifier(BonusRatio);
		}
		break;
	}
	ModifiedStatComponent = Stats;
}

void UMVStatModifierPerStackBehavior::RemoveModifier()
{
	if (UMVStatComponent* Stats = ModifiedStatComponent.Get(); Stats && ModifierHandle.IsValid())
	{
		switch (Stat)
		{
		case EMVPerStackStat::AttackSpeed:
			Stats->RemoveAttackSpeedModifier(ModifierHandle);
			break;
		case EMVPerStackStat::MoveSpeed:
			Stats->RemoveMoveSpeedModifier(ModifierHandle);
			break;
		}
	}
	ModifierHandle.Invalidate();
	ModifiedStatComponent.Reset();
}
