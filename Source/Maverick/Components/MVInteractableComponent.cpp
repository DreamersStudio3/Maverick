#include "Components/MVInteractableComponent.h"

#include "Engine/GameInstance.h"
#include "Interaction/MVInteractionFlowDataAsset.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVDialogueWindow.h"
#include "UI/Window/MVInteractionMenuWindow.h"

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

void UMVInteractableComponent::SetUseInteractionDefinition(const bool bInUseInteractionDefinition)
{
	bUseInteractionDefinition = bInUseInteractionDefinition;
}

void UMVInteractableComponent::SetInteractionDefinition(const FMVInteractionDefinition& InInteractionDefinition)
{
	InteractionDefinition = InInteractionDefinition;
}

void UMVInteractableComponent::SetInteractionFlowAsset(UMVInteractionFlowDataAsset* InInteractionFlowAsset)
{
	InteractionFlowAsset = InInteractionFlowAsset;
}

void UMVInteractableComponent::FinishInteractionAction()
{
	if (!bConfiguredInteractionRunning || !bWaitingForConfiguredStep)
	{
		return;
	}

	const FMVInteractionStepConfig* Step = FindInteractionStep(ActiveStepId);
	if (!Step || Step->StepType != EMVInteractionStepType::Action)
	{
		return;
	}

	const FName ActionName = Step->ActionName.IsNone() ? Step->StepId : Step->ActionName;
	OnInteractionActionCompleted.Broadcast(ActiveInteractor.Get(), this, Step->StepId, ActionName);
	CompleteConfiguredStep(Step->NextStepId);
}

bool UMVInteractableComponent::CanInteract_Implementation(AActor* Interactor) const
{
	return bCanInteract && !bConfiguredInteractionRunning;
}

void UMVInteractableComponent::Interact_Implementation(AActor* Interactor)
{
	if (!bCanInteract)
	{
		return;
	}

	if (bUseInteractionDefinition)
	{
		ExecuteConfiguredInteraction(Interactor);
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

void UMVInteractableComponent::ExecuteConfiguredInteraction(AActor* Interactor)
{
	if (bConfiguredInteractionRunning)
	{
		return;
	}

	BeginConfiguredInteractionSession(Interactor);
	if (!ExecuteConfiguredStep(ResolveStartStepId()))
	{
		EndConfiguredInteractionSession();
	}
}

void UMVInteractableComponent::BeginConfiguredInteractionSession(AActor* Interactor)
{
	bConfiguredInteractionRunning = true;
	bWaitingForConfiguredStep = false;
	bHasPendingStepAfterMenuClose = false;
	ActiveInteractor = Interactor;
	ActiveStepId = NAME_None;
	PendingStepAfterMenuClose = NAME_None;

	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->BeginInteractionSession(this);
	}
}

void UMVInteractableComponent::EndConfiguredInteractionSession()
{
	if (ActiveConfiguredDialogueWindow)
	{
		ActiveConfiguredDialogueWindow->OnDialogueWindowClosed.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredDialogueClosed);
	}
	if (ActiveConfiguredPopup)
	{
		ActiveConfiguredPopup->OnPopupClosed.RemoveDynamic(this, &UMVInteractableComponent::HandleConfiguredPopupClosed);
	}
	if (ActiveConfiguredMenuWindow)
	{
		ActiveConfiguredMenuWindow->OnInteractionMenuClosed.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredMenuClosed);
		ActiveConfiguredMenuWindow->OnInteractionMenuActionSelected.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredMenuActionSelected);
	}
	if (ActiveConfiguredWindow)
	{
		ActiveConfiguredWindow->OnWindowDeactivated.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredWindowDeactivated);
	}

	ActiveConfiguredDialogueWindow = nullptr;
	ActiveConfiguredPopup = nullptr;
	ActiveConfiguredMenuWindow = nullptr;
	ActiveConfiguredWindow = nullptr;

	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->EndInteractionSession(this);
	}

	ActiveInteractor = nullptr;
	ActiveStepId = NAME_None;
	PendingStepAfterMenuClose = NAME_None;
	bWaitingForConfiguredStep = false;
	bConfiguredInteractionRunning = false;
	bHasPendingStepAfterMenuClose = false;
}

bool UMVInteractableComponent::ExecuteConfiguredStep(const FName StepId)
{
	const FMVInteractionStepConfig* Step = FindInteractionStep(StepId);
	if (!Step)
	{
		return false;
	}

	ActiveStepId = Step->StepId;
	bWaitingForConfiguredStep = false;
	bHasPendingStepAfterMenuClose = false;
	PendingStepAfterMenuClose = NAME_None;

	switch (Step->StepType)
	{
	case EMVInteractionStepType::Dialogue:
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredDialogueWindow = Step->DialogueId.IsNone()
				? UISubsystem->ShowDialogueWindowTextWithTiming(
					Step->DialogueText,
					Step->DialogueDuration,
					Step->DialogueMinimumSkipDelay)
				: UISubsystem->ShowDialogueWindowById(Step->DialogueId);
			if (ActiveConfiguredDialogueWindow)
			{
				bWaitingForConfiguredStep = true;
				ActiveConfiguredDialogueWindow->OnDialogueWindowClosed.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredDialogueClosed);
				return true;
			}
		}
		CompleteConfiguredStep(Step->NextStepId);
		return true;

	case EMVInteractionStepType::Action:
	{
		const FName ActionName = Step->ActionName.IsNone() ? Step->StepId : Step->ActionName;
		bWaitingForConfiguredStep = true;
		OnInteractionActionRequested.Broadcast(ActiveInteractor.Get(), this, Step->StepId, ActionName);
		return true;
	}

	case EMVInteractionStepType::WarningPopup:
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredPopup = Step->WarningMessageId.IsNone()
				? UISubsystem->ShowPopupMessageText(Step->WarningMessageText, Step->WarningDuration)
				: UISubsystem->ShowPopupMessageById(Step->WarningMessageId);
			if (ActiveConfiguredPopup)
			{
				bWaitingForConfiguredStep = true;
				ActiveConfiguredPopup->OnPopupClosed.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredPopupClosed);
				return true;
			}
		}
		CompleteConfiguredStep(Step->NextStepId);
		return true;

	case EMVInteractionStepType::Menu:
	case EMVInteractionStepType::Choice:
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredMenuWindow = UISubsystem->ShowInteractionMenu(Step->MenuData, this);
			if (ActiveConfiguredMenuWindow)
			{
				bWaitingForConfiguredStep = true;
				ActiveConfiguredMenuWindow->OnInteractionMenuClosed.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredMenuClosed);
				ActiveConfiguredMenuWindow->OnInteractionMenuActionSelected.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredMenuActionSelected);
				return true;
			}
		}
		CompleteConfiguredStep(Step->NextStepId);
		return true;

	case EMVInteractionStepType::Window:
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredWindow = Cast<UMVWindowBase>(UISubsystem->PushWindowByClass(Step->WindowClass));
			if (ActiveConfiguredWindow)
			{
				bWaitingForConfiguredStep = true;
				ActiveConfiguredWindow->OnWindowDeactivated.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredWindowDeactivated);
				return true;
			}
		}
		CompleteConfiguredStep(Step->NextStepId);
		return true;

	case EMVInteractionStepType::None:
	default:
		CompleteConfiguredStep(Step->NextStepId);
		return true;
	}
}

void UMVInteractableComponent::CompleteConfiguredStep(const FName NextStepId)
{
	bWaitingForConfiguredStep = false;

	if (ActiveConfiguredDialogueWindow)
	{
		ActiveConfiguredDialogueWindow->OnDialogueWindowClosed.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredDialogueClosed);
		ActiveConfiguredDialogueWindow = nullptr;
	}
	if (ActiveConfiguredPopup)
	{
		ActiveConfiguredPopup->OnPopupClosed.RemoveDynamic(this, &UMVInteractableComponent::HandleConfiguredPopupClosed);
		ActiveConfiguredPopup = nullptr;
	}
	if (ActiveConfiguredMenuWindow)
	{
		ActiveConfiguredMenuWindow->OnInteractionMenuClosed.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredMenuClosed);
		ActiveConfiguredMenuWindow->OnInteractionMenuActionSelected.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredMenuActionSelected);
		ActiveConfiguredMenuWindow = nullptr;
	}
	if (ActiveConfiguredWindow)
	{
		ActiveConfiguredWindow->OnWindowDeactivated.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredWindowDeactivated);
		ActiveConfiguredWindow = nullptr;
	}

	if (NextStepId.IsNone())
	{
		EndConfiguredInteractionSession();
		return;
	}

	if (!ExecuteConfiguredStep(NextStepId))
	{
		EndConfiguredInteractionSession();
	}
}

FName UMVInteractableComponent::ResolveStartStepId() const
{
	const FMVInteractionDefinition& Definition = ResolveInteractionDefinition();
	if (!Definition.StartStepId.IsNone())
	{
		return Definition.StartStepId;
	}

	return Definition.Steps.IsEmpty() ? NAME_None : Definition.Steps[0].StepId;
}

const FMVInteractionDefinition& UMVInteractableComponent::ResolveInteractionDefinition() const
{
	return InteractionFlowAsset ? InteractionFlowAsset->GetInteractionDefinition() : InteractionDefinition;
}

FName UMVInteractableComponent::ResolveStepTransition(
	const FMVInteractionStepConfig& Step,
	const FName TriggerName) const
{
	for (const FMVInteractionStepTransition& Transition : Step.Transitions)
	{
		if (Transition.TriggerName == TriggerName)
		{
			return Transition.NextStepId;
		}
	}

	return Step.NextStepId;
}

const FMVInteractionStepConfig* UMVInteractableComponent::FindInteractionStep(const FName StepId) const
{
	const FMVInteractionDefinition& Definition = ResolveInteractionDefinition();
	return Definition.Steps.FindByPredicate([StepId](const FMVInteractionStepConfig& Step)
	{
		return Step.StepId == StepId;
	});
}

UMVUISubsystem* UMVInteractableComponent::GetUISubsystem() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
}

void UMVInteractableComponent::HandleConfiguredDialogueClosed(UMVDialogueWindow* ClosedDialogueWindow)
{
	if (!ClosedDialogueWindow || ClosedDialogueWindow != ActiveConfiguredDialogueWindow)
	{
		return;
	}

	const FMVInteractionStepConfig* Step = FindInteractionStep(ActiveStepId);
	CompleteConfiguredStep(Step ? Step->NextStepId : NAME_None);
}

void UMVInteractableComponent::HandleConfiguredPopupClosed(UMVPopupBase* ClosedPopup)
{
	if (!ClosedPopup || ClosedPopup != ActiveConfiguredPopup)
	{
		return;
	}

	const FMVInteractionStepConfig* Step = FindInteractionStep(ActiveStepId);
	CompleteConfiguredStep(Step ? Step->NextStepId : NAME_None);
}

void UMVInteractableComponent::HandleConfiguredMenuClosed(UMVInteractionMenuWindow* ClosedMenuWindow)
{
	if (!ClosedMenuWindow || ClosedMenuWindow != ActiveConfiguredMenuWindow)
	{
		return;
	}

	ActiveConfiguredMenuWindow->OnInteractionMenuClosed.RemoveDynamic(
		this,
		&UMVInteractableComponent::HandleConfiguredMenuClosed);
	ActiveConfiguredMenuWindow->OnInteractionMenuActionSelected.RemoveDynamic(
		this,
		&UMVInteractableComponent::HandleConfiguredMenuActionSelected);
	ActiveConfiguredMenuWindow = nullptr;

	const FMVInteractionStepConfig* Step = FindInteractionStep(ActiveStepId);
	const FName NextStepId = bHasPendingStepAfterMenuClose
		? PendingStepAfterMenuClose
		: (Step ? Step->NextStepId : NAME_None);
	bHasPendingStepAfterMenuClose = false;
	PendingStepAfterMenuClose = NAME_None;
	CompleteConfiguredStep(NextStepId);
}

void UMVInteractableComponent::HandleConfiguredMenuActionSelected(UObject* SourceObject, FName ActionName)
{
	if (SourceObject && SourceObject != this)
	{
		return;
	}

	const FMVInteractionStepConfig* Step = FindInteractionStep(ActiveStepId);
	if (!Step)
	{
		return;
	}

	OnInteractionMenuActionRequested.Broadcast(ActiveInteractor.Get(), this, Step->StepId, ActionName);
	PendingStepAfterMenuClose = ResolveStepTransition(*Step, ActionName);
	bHasPendingStepAfterMenuClose = true;

	if (ActiveConfiguredMenuWindow)
	{
		ActiveConfiguredMenuWindow->DeactivateWidgetWithFade();
	}
	else
	{
		const FName NextStepId = PendingStepAfterMenuClose;
		bHasPendingStepAfterMenuClose = false;
		PendingStepAfterMenuClose = NAME_None;
		CompleteConfiguredStep(NextStepId);
	}
}

void UMVInteractableComponent::HandleConfiguredWindowDeactivated(UMVWindowBase* Window)
{
	if (!Window || Window != ActiveConfiguredWindow)
	{
		return;
	}

	const FMVInteractionStepConfig* Step = FindInteractionStep(ActiveStepId);
	CompleteConfiguredStep(Step ? Step->NextStepId : NAME_None);
}
