#include "Components/MVInteractableComponent.h"

UMVInteractableComponent::UMVInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PromptText = NSLOCTEXT("MaverickInteraction", "DefaultInteractPrompt", "Interact");
}

void UMVInteractableComponent::SetCanInteract(bool bInCanInteract)
{
	bCanInteract = bInCanInteract;
}

void UMVInteractableComponent::SetPromptText(FText InPromptText)
{
	PromptText = InPromptText;
}

bool UMVInteractableComponent::CanInteract_Implementation(AActor* Interactor) const
{
	return bCanInteract;
}

void UMVInteractableComponent::Interact_Implementation(AActor* Interactor)
{
	if (!bCanInteract)
	{
		return;
	}

	OnInteractionRequested.Broadcast(Interactor, this);
}

FText UMVInteractableComponent::GetInteractionPromptText_Implementation(AActor* Interactor) const
{
	return PromptText;
}

int32 UMVInteractableComponent::GetInteractionPriority_Implementation(AActor* Interactor) const
{
	return InteractionPriority;
}
