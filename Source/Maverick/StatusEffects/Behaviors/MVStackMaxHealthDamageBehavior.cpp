#include "StatusEffects/Behaviors/MVStackMaxHealthDamageBehavior.h"

#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDamageUtility.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "Struct/MVHitTypes.h"

void UMVStackMaxHealthDamageBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	TryTriggerDamage(Instance, 0);
}

void UMVStackMaxHealthDamageBehavior::OnUpdated_Implementation(const FMVStatusEffectInstance& Instance, const int32 PreviousStacks)
{
	TryTriggerDamage(Instance, PreviousStacks);
}

bool UMVStackMaxHealthDamageBehavior::TryTriggerDamage(const FMVStatusEffectInstance& Instance,	const int32 PreviousStacks)
{
	const int32 SafeRequiredStacks = FMath::Max(1, RequiredStacks);

	if (PreviousStacks >= SafeRequiredStacks)
	{
		return false;
	}

	if (Instance.CurrentStacks < SafeRequiredStacks)
	{
		return false;
	}

	if (Instance.Handle != GetEffectHandle())
	{
		return false;
	}

	UMVStatusEffectComponent* StatusEffectComponent = GetOwningStatusEffectComponent();

	if (!IsValid(StatusEffectComponent))
	{
		return false;
	}

	if (::IsValid(TriggerBlockingEffect.Get()) &&
		StatusEffectComponent->HasStatusEffect(TriggerBlockingEffect.Get(), Instance.SourceActor.Get()))
	{
		return false;
	}

	if (!MVStatusEffectDamage::ApplyTargetMaxHealthDamage(
		StatusEffectComponent,
		Instance.SourceActor.Get(),
		StatusEffectComponent->GetOwner(),
		TargetMaxHealthDamageRatio))
	{
		return false;
	}

	if (::IsValid(StatusEffectAppliedOnTrigger.Get()))
	{
		FMVStatusEffectSpec TriggeredEffectSpec;
		TriggeredEffectSpec.Definition = StatusEffectAppliedOnTrigger;
		TriggeredEffectSpec.SourceActor = Instance.SourceActor;
		TriggeredEffectSpec.StackDelta = 1;

		StatusEffectComponent->ApplyStatusEffect(TriggeredEffectSpec);
	}

	if (!bKeepOwningEffectAfterTrigger)
	{
		StatusEffectComponent->RemoveStatusEffect(GetEffectHandle(), EMVStatusEffectRemovalReason::Consumed);
	}

	return true;
}
