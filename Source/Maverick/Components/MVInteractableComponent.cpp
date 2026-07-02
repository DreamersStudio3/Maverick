#include "Components/MVInteractableComponent.h"

#include "Engine/GameInstance.h"
#include "Interaction/MVInteractionFlowDataAsset.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVDialogueWindow.h"
#include "UI/Window/MVInteractionMenuWindow.h"

namespace
{
FName MVInteractableResolveActionStepName(const UMVInteractionActionStepData& Step)
{
	return Step.ActionName.IsNone() ? Step.StepId : Step.ActionName;
}
}

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

	const UMVInteractionStepData* Step = FindInteractionStep(ActiveStepId);
	const UMVInteractionActionStepData* ActionStep = Cast<UMVInteractionActionStepData>(Step);
	if (!ActionStep)
	{
		return;
	}

	const FName ActionName = MVInteractableResolveActionStepName(*ActionStep);
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
	const UMVInteractionStepData* Step = FindInteractionStep(StepId);
	if (!Step)
	{
		return false;
	}

	ActiveStepId = Step->StepId;
	bWaitingForConfiguredStep = false;
	bHasPendingStepAfterMenuClose = false;
	PendingStepAfterMenuClose = NAME_None;

	if (const UMVInteractionDialogueStepData* DialogueStep = Cast<UMVInteractionDialogueStepData>(Step))
	{
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredDialogueWindow = DialogueStep->DialogueId.IsNone()
				? UISubsystem->ShowDialogueWindowTextWithTiming(
					DialogueStep->DialogueText,
					DialogueStep->DialogueDuration,
					DialogueStep->DialogueMinimumSkipDelay)
				: UISubsystem->ShowDialogueWindowById(DialogueStep->DialogueId);
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
	}

	if (const UMVInteractionActionStepData* ActionStep = Cast<UMVInteractionActionStepData>(Step))
	{
		const FName ActionName = MVInteractableResolveActionStepName(*ActionStep);
		bWaitingForConfiguredStep = true;
		OnInteractionActionRequested.Broadcast(ActiveInteractor.Get(), this, Step->StepId, ActionName);
		return true;
	}

	if (const UMVInteractionWarningPopupStepData* WarningStep = Cast<UMVInteractionWarningPopupStepData>(Step))
	{
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredPopup = WarningStep->WarningMessageId.IsNone()
				? UISubsystem->ShowPopupMessageText(WarningStep->WarningMessageText, WarningStep->WarningDuration)
				: UISubsystem->ShowPopupMessageById(WarningStep->WarningMessageId);
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
	}

	if (const UMVInteractionSelectionStepData* SelectionStep = Cast<UMVInteractionSelectionStepData>(Step))
	{
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredMenuWindow = UISubsystem->ShowInteractionMenu(SelectionStep->MenuData, this);
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
	}

	if (const UMVInteractionWindowStepData* WindowStep = Cast<UMVInteractionWindowStepData>(Step))
	{
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredWindow = Cast<UMVWindowBase>(UISubsystem->PushWindowByClass(WindowStep->WindowClass));
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
	}

	CompleteConfiguredStep(Step->NextStepId);
	return true;
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
	if (InteractionFlowAsset && !InteractionFlowAsset->GetStartStepId().IsNone())
	{
		return InteractionFlowAsset->GetStartStepId();
	}

	if (!InteractionFlowAsset && !InlineStartStepId.IsNone())
	{
		return InlineStartStepId;
	}

	for (const TObjectPtr<UMVInteractionStepData>& Step : ResolveInteractionSteps())
	{
		if (Step)
		{
			return Step->StepId;
		}
	}

	return NAME_None;
}

const TArray<TObjectPtr<UMVInteractionStepData>>& UMVInteractableComponent::ResolveInteractionSteps() const
{
	return InteractionFlowAsset ? InteractionFlowAsset->GetSteps() : InlineSteps;
}

FName UMVInteractableComponent::ResolveStepTransition(
	const UMVInteractionSelectionStepData& Step,
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

const UMVInteractionStepData* UMVInteractableComponent::FindInteractionStep(const FName StepId) const
{
	const TObjectPtr<UMVInteractionStepData>* FoundStep =
		ResolveInteractionSteps().FindByPredicate([StepId](const TObjectPtr<UMVInteractionStepData>& Step)
	{
		return Step && Step->StepId == StepId;
	});
	return FoundStep ? FoundStep->Get() : nullptr;
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

	const UMVInteractionStepData* Step = FindInteractionStep(ActiveStepId);
	CompleteConfiguredStep(Step ? Step->NextStepId : NAME_None);
}

void UMVInteractableComponent::HandleConfiguredPopupClosed(UMVPopupBase* ClosedPopup)
{
	if (!ClosedPopup || ClosedPopup != ActiveConfiguredPopup)
	{
		return;
	}

	const UMVInteractionStepData* Step = FindInteractionStep(ActiveStepId);
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

	const UMVInteractionStepData* Step = FindInteractionStep(ActiveStepId);
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

	const UMVInteractionStepData* Step = FindInteractionStep(ActiveStepId);
	const UMVInteractionSelectionStepData* SelectionStep = Cast<UMVInteractionSelectionStepData>(Step);
	if (!SelectionStep)
	{
		return;
	}

	OnInteractionMenuActionRequested.Broadcast(ActiveInteractor.Get(), this, Step->StepId, ActionName);
	PendingStepAfterMenuClose = ResolveStepTransition(*SelectionStep, ActionName);
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

	const UMVInteractionStepData* Step = FindInteractionStep(ActiveStepId);
	CompleteConfiguredStep(Step ? Step->NextStepId : NAME_None);
}
