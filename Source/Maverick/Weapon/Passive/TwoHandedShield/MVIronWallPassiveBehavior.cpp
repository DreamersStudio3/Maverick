#include "Weapon/Passive/TwoHandedShield/MVIronWallPassiveBehavior.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Components/MVStatusEffectComponent.h"

void UMVIronWallPassiveBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	if (Instance.Handle != GetEffectHandle()
		|| BoundStatComponent.IsValid()
		|| DefenceModifierHandle.IsValid()
		|| MaxHPModifierHandle.IsValid())
	{
		return;
	}

	UMVStatusEffectComponent* Effects = GetOwningStatusEffectComponent();
	AMVCharacterBase* Owner = IsValid(Effects) ? Cast<AMVCharacterBase>(Effects->GetOwner()) : nullptr;
	UMVStatComponent* Stats = IsValid(Owner) ? Owner->StatComponent.Get() : nullptr;
	if (!IsValid(Stats) || Instance.SourceActor.Get() != Owner)
	{
		return;
	}

	const FGuid NewDefenceHandle = Stats->AddDefenceModifier(DefenceBonusRatio);
	if (!NewDefenceHandle.IsValid())
	{
		return;
	}
	BoundStatComponent = Stats;
	DefenceModifierHandle = NewDefenceHandle;

	const FGuid NewMaxHPHandle = Stats->AddMaxHPModifier(MaxHPBonusRatio);
	if (!NewMaxHPHandle.IsValid())
	{
		Stats->RemoveDefenceModifier(NewDefenceHandle);
		DefenceModifierHandle = FGuid();
		BoundStatComponent.Reset();
		return;
	}

	// 최대 체력 변경 통지 중 효과가 제거될 수 있으므로, 방금 등록한 보정도 되돌린다.
	const FMVStatusEffectHandle ActiveHandle = GetEffectHandle();
	const bool bEffectStillActive = IsValid(Effects)
		&& Effects->GetActiveEffects().ContainsByPredicate(
		[ActiveHandle, Owner](const FMVStatusEffectInstance& Candidate)
		{
			return Candidate.Handle == ActiveHandle && Candidate.SourceActor.Get() == Owner;
		});
	if (!BoundStatComponent.IsValid() || !bEffectStillActive)
	{
		Stats->RemoveMaxHPModifier(NewMaxHPHandle);
		Stats->RemoveDefenceModifier(NewDefenceHandle);
		DefenceModifierHandle = FGuid();
		BoundStatComponent.Reset();
		return;
	}

	MaxHPModifierHandle = NewMaxHPHandle;
}

void UMVIronWallPassiveBehavior::OnRemoved_Implementation(
	const FMVStatusEffectInstance& /*Instance*/,
	EMVStatusEffectRemovalReason /*RemovalReason*/)
{
	if (UMVStatComponent* Stats = BoundStatComponent.Get())
	{
		if (MaxHPModifierHandle.IsValid())
		{
			Stats->RemoveMaxHPModifier(MaxHPModifierHandle);
		}
		if (DefenceModifierHandle.IsValid())
		{
			Stats->RemoveDefenceModifier(DefenceModifierHandle);
		}
	}

	DefenceModifierHandle = FGuid();
	MaxHPModifierHandle = FGuid();
	BoundStatComponent.Reset();
}
