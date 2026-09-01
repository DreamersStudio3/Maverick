// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusEffects/Behaviors/MVResetStatusEffectStackBehavior.h"

#include "Components/MVStatusEffectComponent.h"
#include "StatusEffects/MVStatusEffectDefinition.h"

void UMVResetStatusEffectStackBehavior::OnRemoved_Implementation(const FMVStatusEffectInstance& Instance,
                                                                 EMVStatusEffectRemovalReason RemovalReason)
{
	Super::OnRemoved_Implementation(Instance, RemovalReason);

	if (RemovalReason != EMVStatusEffectRemovalReason::Expired)
	{
		return;
	}

	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	if (!::IsValid(TargetDefinition.Get()))
	{
		return;
	}

	UMVStatusEffectComponent* StatusEffectComponent = GetOwningStatusEffectComponent();

	if (!::IsValid(StatusEffectComponent))
	{
		return;
	}

	const FMVStatusEffectHandle TargetHandle =
		StatusEffectComponent->FindStatusEffectHandle(TargetDefinition.Get(), Instance.SourceActor.Get());

	if (!TargetHandle.IsValid())
	{
		return;
	}

	StatusEffectComponent->SetStatusEffectStacks(TargetHandle, ResetStacks);
}
