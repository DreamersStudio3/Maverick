#include "Compatibility/MVLegacyComponentShims.h"

UMVDodgeComponent::UMVDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVDodgeComponent::PrepareDodgeAction()
{
}

void UMVDodgeComponent::UpdateBufferedDodgeMovementInput(const FVector&)
{
}

UMVInteractionDetectorComponent::UMVInteractionDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	DefaultPromptText = NSLOCTEXT("MaverickInteraction", "DefaultInteractPrompt", "Interact");
	InteractionObjectChannels = {
		ECC_WorldDynamic,
		ECC_Pawn,
		ECC_PhysicsBody
	};
}

void UMVInteractionDetectorComponent::SetInteractionDetectionEnabled(const bool bInEnabled)
{
	bDetectionEnabled = bInEnabled;
}

void UMVInteractionDetectorComponent::RefreshInteractable()
{
}

void UMVInteractionDetectorComponent::ClearFocusedInteractable()
{
}

bool UMVInteractionDetectorComponent::TryInteract()
{
	return false;
}

bool UMVInteractionDetectorComponent::SelectNextInteractable()
{
	return false;
}

bool UMVInteractionDetectorComponent::SelectPreviousInteractable()
{
	return false;
}

UObject* UMVInteractionDetectorComponent::GetFocusedInteractable() const
{
	return nullptr;
}

bool UMVInteractionDetectorComponent::HasFocusedInteractable() const
{
	return false;
}

int32 UMVInteractionDetectorComponent::GetInteractableCandidateCount() const
{
	return 0;
}

int32 UMVInteractionDetectorComponent::GetSelectedInteractableCandidateIndex() const
{
	return INDEX_NONE;
}
