#include "StatusEffects/MVStatusEffectBehavior.h"

#include "Components/MVStatusEffectComponent.h"

UWorld* UMVStatusEffectBehavior::GetWorld() const
{
	const UMVStatusEffectComponent* Component = OwningComponent.Get();

	return Component ? Component->GetWorld() : nullptr;
}

void UMVStatusEffectBehavior::InitializeBehavior(UMVStatusEffectComponent* InOwningComponent,
                                                 FMVStatusEffectHandle InEffectHandle)
{
	OwningComponent = InOwningComponent;
	EffectHandle = InEffectHandle;
}

void UMVStatusEffectBehavior::DeinitializeBehavior()
{
	OwningComponent.Reset();
	EffectHandle.Reset();
}

void UMVStatusEffectBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
}

void UMVStatusEffectBehavior::OnUpdated_Implementation(const FMVStatusEffectInstance& Instance, int32 PreviousStacks)
{
}

void UMVStatusEffectBehavior::OnRemoved_Implementation(const FMVStatusEffectInstance& Instance,
                                                       EMVStatusEffectRemovalReason RemovalReason)
{
}
