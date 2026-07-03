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
bool MVInteractableHasActionStepRow(const FMVInteractionActionStepData& Step)
{
	return Step.ActionRow.DataTable && !Step.ActionRow.RowName.IsNone();
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

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	const FMVInteractionActionStepData* ActionStep = StepInstance ? StepInstance->GetPtr<FMVInteractionActionStepData>() : nullptr;
	if (!Step || !ActionStep || !MVInteractableHasActionStepRow(*ActionStep))
	{
		return;
	}

	OnInteractionActionCompleted.Broadcast(
		ActiveInteractor.Get(),
		this,
		Step->StepId,
		ActionStep->ActionRow,
		ActionStep->StartSection);
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
	ActiveStepId = FGameplayTag();
	PendingStepAfterMenuClose = FGameplayTag();

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
	ActiveStepId = FGameplayTag();
	PendingStepAfterMenuClose = FGameplayTag();
	bWaitingForConfiguredStep = false;
	bConfiguredInteractionRunning = false;
	bHasPendingStepAfterMenuClose = false;
}

bool UMVInteractableComponent::ExecuteConfiguredStep(const FGameplayTag StepId)
{
	const FInstancedStruct* StepInstance = FindInteractionStep(StepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	if (!Step)
	{
		return false;
	}

	ActiveStepId = Step->StepId;
	bWaitingForConfiguredStep = false;
	bHasPendingStepAfterMenuClose = false;
	PendingStepAfterMenuClose = FGameplayTag();

	if (const FMVInteractionDialogueStepData* DialogueStep = StepInstance->GetPtr<FMVInteractionDialogueStepData>())
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

	if (const FMVInteractionActionStepData* ActionStep = StepInstance->GetPtr<FMVInteractionActionStepData>())
	{
		if (!MVInteractableHasActionStepRow(*ActionStep))
		{
			return false;
		}

		bWaitingForConfiguredStep = true;
		OnInteractionActionRequested.Broadcast(
			ActiveInteractor.Get(),
			this,
			Step->StepId,
			ActionStep->ActionRow,
			ActionStep->StartSection);
		return true;
	}

	if (const FMVInteractionWarningPopupStepData* WarningStep = StepInstance->GetPtr<FMVInteractionWarningPopupStepData>())
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

	if (const FMVInteractionSelectionStepData* SelectionStep = StepInstance->GetPtr<FMVInteractionSelectionStepData>())
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

	if (const FMVInteractionWindowStepData* WindowStep = StepInstance->GetPtr<FMVInteractionWindowStepData>())
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

void UMVInteractableComponent::CompleteConfiguredStep(const FGameplayTag NextStepId)
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

	if (!NextStepId.IsValid())
	{
		EndConfiguredInteractionSession();
		return;
	}

	if (!ExecuteConfiguredStep(NextStepId))
	{
		EndConfiguredInteractionSession();
	}
}

FGameplayTag UMVInteractableComponent::ResolveStartStepId() const
{
	if (InteractionFlowAsset && InteractionFlowAsset->GetStartStepId().IsValid())
	{
		return InteractionFlowAsset->GetStartStepId();
	}

	if (!InteractionFlowAsset && InlineStartStepId.IsValid())
	{
		return InlineStartStepId;
	}

	for (const FInstancedStruct& StepInstance : ResolveInteractionSteps())
	{
		if (const FMVInteractionStepData* Step = StepInstance.GetPtr<FMVInteractionStepData>())
		{
			return Step->StepId;
		}
	}

	return FGameplayTag();
}

const TArray<FInstancedStruct>& UMVInteractableComponent::ResolveInteractionSteps() const
{
	return InteractionFlowAsset ? InteractionFlowAsset->GetSteps() : InlineSteps;
}

FGameplayTag UMVInteractableComponent::ResolveStepTransition(
	const FMVInteractionSelectionStepData& Step,
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

const FInstancedStruct* UMVInteractableComponent::FindInteractionStep(const FGameplayTag StepId) const
{
	return ResolveInteractionSteps().FindByPredicate([StepId](const FInstancedStruct& StepInstance)
	{
		const FMVInteractionStepData* Step = StepInstance.GetPtr<FMVInteractionStepData>();
		return Step && Step->StepId == StepId;
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

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	CompleteConfiguredStep(Step ? Step->NextStepId : FGameplayTag());
}

void UMVInteractableComponent::HandleConfiguredPopupClosed(UMVPopupBase* ClosedPopup)
{
	if (!ClosedPopup || ClosedPopup != ActiveConfiguredPopup)
	{
		return;
	}

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	CompleteConfiguredStep(Step ? Step->NextStepId : FGameplayTag());
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

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	const FGameplayTag NextStepId = bHasPendingStepAfterMenuClose
		? PendingStepAfterMenuClose
		: (Step ? Step->NextStepId : FGameplayTag());
	bHasPendingStepAfterMenuClose = false;
	PendingStepAfterMenuClose = FGameplayTag();
	CompleteConfiguredStep(NextStepId);
}

void UMVInteractableComponent::HandleConfiguredMenuActionSelected(UObject* SourceObject, FName ActionName)
{
	if (SourceObject && SourceObject != this)
	{
		return;
	}

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	const FMVInteractionSelectionStepData* SelectionStep =
		StepInstance ? StepInstance->GetPtr<FMVInteractionSelectionStepData>() : nullptr;
	if (!Step || !SelectionStep)
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
		const FGameplayTag NextStepId = PendingStepAfterMenuClose;
		bHasPendingStepAfterMenuClose = false;
		PendingStepAfterMenuClose = FGameplayTag();
		CompleteConfiguredStep(NextStepId);
	}
}

void UMVInteractableComponent::HandleConfiguredWindowDeactivated(UMVWindowBase* Window)
{
	if (!Window || Window != ActiveConfiguredWindow)
	{
		return;
	}

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	CompleteConfiguredStep(Step ? Step->NextStepId : FGameplayTag());
}
