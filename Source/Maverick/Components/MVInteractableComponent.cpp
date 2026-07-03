#include "Components/MVInteractableComponent.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Engine/GameInstance.h"
#include "Interaction/MVInteractionFlowDataAsset.h"
#include "Struct/MVHitTypes.h"
#include "Tags/MVGameplayTags.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVDialogueWindow.h"
#include "UI/Window/MVInteractionChoiceWindow.h"
#include "UI/Window/MVInteractionMenuWindow.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVInteractableComponent, Log, All);

namespace
{
bool MVInteractableCommandWaitsForCompletion(const FMVInteractionCommandRequest& CommandRequest)
{
	return CommandRequest.bWaitForCompletion;
}

bool MVInteractableIsHitReactionApplyDamageEventTag(const FGameplayTag EventTag)
{
	return EventTag == MVGameplayTags::Interaction_Command_Event_HitReaction_ApplyDamage
		|| EventTag == MVGameplayTags::Event_HitReaction_ApplyDamage;
}

TArray<FString> MVInteractableParseHitReactionCommandName(const FName CommandName)
{
	FString CommandString = CommandName.ToString();
	CommandString.ReplaceInline(TEXT("_"), TEXT("."));

	TArray<FString> Tokens;
	CommandString.ParseIntoArray(Tokens, TEXT("."), true);
	return Tokens;
}

EMVActionHitReactionType MVInteractableResolveHitReactionTypeFromName(const FName CommandName)
{
	const TArray<FString> Tokens = MVInteractableParseHitReactionCommandName(CommandName);
	const FString TypeToken = Tokens.IsEmpty() ? FString() : Tokens[0];

	if (TypeToken.Equals(TEXT("SmallHit"), ESearchCase::IgnoreCase)
		|| TypeToken.Equals(TEXT("SH"), ESearchCase::IgnoreCase))
	{
		return EMVActionHitReactionType::SmallHit;
	}

	if (TypeToken.Equals(TEXT("LargeHit"), ESearchCase::IgnoreCase)
		|| TypeToken.Equals(TEXT("LH"), ESearchCase::IgnoreCase))
	{
		return EMVActionHitReactionType::LargeHit;
	}

	if (TypeToken.Equals(TEXT("KnockDown"), ESearchCase::IgnoreCase)
		|| TypeToken.Equals(TEXT("KD"), ESearchCase::IgnoreCase))
	{
		return EMVActionHitReactionType::KnockDown;
	}

	if (TypeToken.Equals(TEXT("Airborne"), ESearchCase::IgnoreCase)
		|| TypeToken.Equals(TEXT("AB"), ESearchCase::IgnoreCase))
	{
		return EMVActionHitReactionType::Airborne;
	}

	return EMVActionHitReactionType::None;
}

FVector MVInteractableResolveHitReactionDirectionFromName(
	const AMVCharacterBase& Victim,
	const FName CommandName)
{
	const TArray<FString> Tokens = MVInteractableParseHitReactionCommandName(CommandName);
	const FString DirectionToken = Tokens.Num() >= 2 ? Tokens.Last() : TEXT("F");

	FVector IncomingDirection = Victim.GetActorForwardVector().GetSafeNormal2D();
	if (DirectionToken.Equals(TEXT("L"), ESearchCase::IgnoreCase)
		|| DirectionToken.Equals(TEXT("Left"), ESearchCase::IgnoreCase))
	{
		IncomingDirection = -Victim.GetActorRightVector().GetSafeNormal2D();
	}
	else if (DirectionToken.Equals(TEXT("R"), ESearchCase::IgnoreCase)
		|| DirectionToken.Equals(TEXT("Right"), ESearchCase::IgnoreCase))
	{
		IncomingDirection = Victim.GetActorRightVector().GetSafeNormal2D();
	}
	else if (DirectionToken.Equals(TEXT("B"), ESearchCase::IgnoreCase)
		|| DirectionToken.Equals(TEXT("Back"), ESearchCase::IgnoreCase))
	{
		IncomingDirection = -Victim.GetActorForwardVector().GetSafeNormal2D();
	}

	return -IncomingDirection;
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

void UMVInteractableComponent::FinishInteractionCommand()
{
	if (!bConfiguredInteractionRunning || !bWaitingForConfiguredCommand)
	{
		return;
	}

	if (bBroadcastingConfiguredCommandRequest)
	{
		bCompleteConfiguredCommandAfterRequest = true;
		return;
	}

	OnInteractionCommandCompleted.Broadcast(
		ActiveInteractor.Get(),
		this,
		ActiveStepId,
		ActiveCommandRequest);

	bWaitingForConfiguredCommand = false;
	++ActiveCommandIndex;
	if (!ExecuteNextConfiguredCommand())
	{
		EndConfiguredInteractionSession();
	}
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
	bWaitingForConfiguredCommand = false;
	bBroadcastingConfiguredCommandRequest = false;
	bCompleteConfiguredCommandAfterRequest = false;
	bHasPendingStepAfterMenuClose = false;
	bHasPendingCommandsAfterMenuClose = false;
	ActiveInteractor = Interactor;
	ActiveStepId = FGameplayTag();
	PendingStepAfterMenuClose = FGameplayTag();
	PendingStepAfterCommands = FGameplayTag();
	ActiveCommandIndex = INDEX_NONE;
	ActiveCommandInstances.Reset();
	PendingCommandsAfterMenuClose.Reset();
	ActiveCommandRequest = FMVInteractionCommandRequest();

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
		ActiveConfiguredMenuWindow->OnInteractionMenuEntrySelected.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredMenuEntrySelected);
	}
	if (ActiveConfiguredChoiceWindow)
	{
		ActiveConfiguredChoiceWindow->OnInteractionChoiceClosed.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredChoiceClosed);
		ActiveConfiguredChoiceWindow->OnInteractionChoiceEntrySelected.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredChoiceEntrySelected);
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
	ActiveConfiguredChoiceWindow = nullptr;
	ActiveConfiguredWindow = nullptr;

	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->EndInteractionSession(this);
	}

	ActiveInteractor = nullptr;
	ActiveStepId = FGameplayTag();
	PendingStepAfterMenuClose = FGameplayTag();
	PendingStepAfterCommands = FGameplayTag();
	ActiveCommandIndex = INDEX_NONE;
	ActiveCommandInstances.Reset();
	PendingCommandsAfterMenuClose.Reset();
	ActiveCommandRequest = FMVInteractionCommandRequest();
	bWaitingForConfiguredStep = false;
	bWaitingForConfiguredCommand = false;
	bBroadcastingConfiguredCommandRequest = false;
	bCompleteConfiguredCommandAfterRequest = false;
	bConfiguredInteractionRunning = false;
	bHasPendingStepAfterMenuClose = false;
	bHasPendingCommandsAfterMenuClose = false;
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
			ActiveConfiguredDialogueWindow = UISubsystem->ShowDialogueWindowByRow(DialogueStep->DialogueRow);
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
		return ExecuteConfiguredCommands(ActionStep->Commands, Step->NextStepId);
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

	if (const FMVInteractionChoiceStepData* ChoiceStep = StepInstance->GetPtr<FMVInteractionChoiceStepData>())
	{
		if (UMVUISubsystem* UISubsystem = GetUISubsystem())
		{
			ActiveConfiguredChoiceWindow = UISubsystem->ShowInteractionChoice(ChoiceStep->ChoiceData, this);
			if (ActiveConfiguredChoiceWindow)
			{
				bWaitingForConfiguredStep = true;
				ActiveConfiguredChoiceWindow->OnInteractionChoiceClosed.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredChoiceClosed);
				ActiveConfiguredChoiceWindow->OnInteractionChoiceEntrySelected.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredChoiceEntrySelected);
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
				ActiveConfiguredMenuWindow->OnInteractionMenuEntrySelected.AddUniqueDynamic(
					this,
					&UMVInteractableComponent::HandleConfiguredMenuEntrySelected);
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
	bWaitingForConfiguredCommand = false;
	bBroadcastingConfiguredCommandRequest = false;
	bCompleteConfiguredCommandAfterRequest = false;

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
		ActiveConfiguredMenuWindow->OnInteractionMenuEntrySelected.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredMenuEntrySelected);
		ActiveConfiguredMenuWindow = nullptr;
	}
	if (ActiveConfiguredChoiceWindow)
	{
		ActiveConfiguredChoiceWindow->OnInteractionChoiceClosed.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredChoiceClosed);
		ActiveConfiguredChoiceWindow->OnInteractionChoiceEntrySelected.RemoveDynamic(
			this,
			&UMVInteractableComponent::HandleConfiguredChoiceEntrySelected);
		ActiveConfiguredChoiceWindow = nullptr;
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

bool UMVInteractableComponent::ExecuteConfiguredCommands(
	const TArray<FInstancedStruct>& Commands,
	const FGameplayTag NextStepId)
{
	if (Commands.IsEmpty())
	{
		CompleteConfiguredStep(NextStepId);
		return true;
	}

	ActiveCommandInstances = Commands;
	ActiveCommandIndex = 0;
	PendingStepAfterCommands = NextStepId;
	ActiveCommandRequest = FMVInteractionCommandRequest();
	bWaitingForConfiguredStep = true;
	bWaitingForConfiguredCommand = false;
	return ExecuteNextConfiguredCommand();
}

bool UMVInteractableComponent::ExecuteNextConfiguredCommand()
{
	while (ActiveCommandInstances.IsValidIndex(ActiveCommandIndex))
	{
		const FInstancedStruct& CommandInstance = ActiveCommandInstances[ActiveCommandIndex];
		const FMVInteractionCommandData* Command = CommandInstance.GetPtr<FMVInteractionCommandData>();
		if (!Command)
		{
			return false;
		}

		ActiveCommandRequest = MakeCommandRequest(CommandInstance);
		if (ActiveCommandRequest.CommandKind == EMVInteractionCommandKind::None)
		{
			return false;
		}

		bWaitingForConfiguredCommand = MVInteractableCommandWaitsForCompletion(ActiveCommandRequest);
		bCompleteConfiguredCommandAfterRequest = false;
		bBroadcastingConfiguredCommandRequest = true;
		OnInteractionCommandRequested.Broadcast(
			ActiveInteractor.Get(),
			this,
			ActiveStepId,
			ActiveCommandRequest);
		const bool bHandledBuiltInCommand = TryHandleBuiltInGameplayEventCommand(ActiveCommandRequest);
		bBroadcastingConfiguredCommandRequest = false;

		if (bWaitingForConfiguredCommand)
		{
			if (!bCompleteConfiguredCommandAfterRequest && !bHandledBuiltInCommand)
			{
				return true;
			}

			bCompleteConfiguredCommandAfterRequest = false;
			OnInteractionCommandCompleted.Broadcast(
				ActiveInteractor.Get(),
				this,
				ActiveStepId,
				ActiveCommandRequest);
			bWaitingForConfiguredCommand = false;
			++ActiveCommandIndex;
			continue;
		}

		OnInteractionCommandCompleted.Broadcast(
			ActiveInteractor.Get(),
			this,
			ActiveStepId,
			ActiveCommandRequest);
		++ActiveCommandIndex;
	}

	const FGameplayTag NextStepId = PendingStepAfterCommands;
	ActiveCommandInstances.Reset();
	ActiveCommandIndex = INDEX_NONE;
	PendingStepAfterCommands = FGameplayTag();
	ActiveCommandRequest = FMVInteractionCommandRequest();
	bWaitingForConfiguredCommand = false;
	bBroadcastingConfiguredCommandRequest = false;
	bCompleteConfiguredCommandAfterRequest = false;
	CompleteConfiguredStep(NextStepId);
	return true;
}

bool UMVInteractableComponent::TryHandleBuiltInGameplayEventCommand(
	const FMVInteractionCommandRequest& CommandRequest)
{
	if (CommandRequest.CommandKind != EMVInteractionCommandKind::GameplayEvent
		|| !MVInteractableIsHitReactionApplyDamageEventTag(CommandRequest.EventTag))
	{
		return false;
	}

	AMVCharacterBase* Victim = Cast<AMVCharacterBase>(ActiveInteractor.Get());
	if (!Victim)
	{
		UE_LOG(
			LogMVInteractableComponent,
			Warning,
			TEXT("Interaction HitReaction.ApplyDamage command ignored. Interactor is not AMVCharacterBase. Owner=%s, EventTag=%s."),
			*GetNameSafe(GetOwner()),
			*CommandRequest.EventTag.ToString());
		return true;
	}

	const float FinalDamage = FMath::Max(0.0f, CommandRequest.Magnitude);
	const EMVActionHitReactionType HitReactionType =
		MVInteractableResolveHitReactionTypeFromName(CommandRequest.Name);

	FMVResolvedHitData HitData;
	HitData.Attacker = Cast<AMVCharacterBase>(GetOwner());
	HitData.Victim = Victim;
	HitData.AttackerCharacterIndexCode = HitData.Attacker
		? HitData.Attacker->GetCharacterIndexCode()
		: FGameplayTag();
	HitData.VictimCharacterIndexCode = Victim->GetCharacterIndexCode();
	HitData.ActionRowName = CommandRequest.Name;
	HitData.CharacterAttackPower = 0.0f;
	HitData.WeaponAttackPower = FinalDamage;
	HitData.VictimDefence = 0.0f;
	HitData.DamageMultiplier = 1.0f;
	HitData.FinalDamage = FinalDamage;
	HitData.GroggyDamage = 0.0f;
	HitData.HitReactionType = HitReactionType;
	HitData.HitLocation = Victim->GetActorLocation();
	HitData.HitDirection = MVInteractableResolveHitReactionDirectionFromName(*Victim, CommandRequest.Name);

	if (UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(this))
	{
		HitResolver->OnHitResolved.Broadcast(HitData);
	}

	Victim->OnHitResolved(HitData);
	return true;
}

FMVInteractionCommandRequest UMVInteractableComponent::MakeCommandRequest(
	const FInstancedStruct& CommandInstance) const
{
	FMVInteractionCommandRequest Request;
	const FMVInteractionCommandData* Command = CommandInstance.GetPtr<FMVInteractionCommandData>();
	if (!Command)
	{
		return Request;
	}

	Request.CommandId = Command->CommandId;
	Request.bWaitForCompletion = Command->bWaitForCompletion;

	if (const FMVInteractionPlayActionCommandData* PlayActionCommand =
		CommandInstance.GetPtr<FMVInteractionPlayActionCommandData>())
	{
		Request.CommandKind = EMVInteractionCommandKind::PlayAction;
		Request.ActionRow = PlayActionCommand->ActionRow;
		Request.StartSection = PlayActionCommand->StartSection;
		return Request;
	}

	if (const FMVInteractionGameplayEventCommandData* EventCommand =
		CommandInstance.GetPtr<FMVInteractionGameplayEventCommandData>())
	{
		Request.CommandKind = EMVInteractionCommandKind::GameplayEvent;
		Request.EventTag = EventCommand->EventTag;
		Request.Tags = EventCommand->Tags;
		Request.Name = EventCommand->Name;
		Request.Magnitude = EventCommand->Magnitude;
		Request.PayloadObject = EventCommand->PayloadObject;
		return Request;
	}

	return Request;
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

FGameplayTag UMVInteractableComponent::ResolveChoiceTransition(
	const FMVInteractionChoiceStepData& Step,
	const FGameplayTag SelectedEntryId) const
{
	for (const FMVInteractionChoiceEntryData& Choice : Step.ChoiceData.Choices)
	{
		if (Choice.ChoiceId == SelectedEntryId)
		{
			return Choice.NextStepId.IsValid() ? Choice.NextStepId : Step.NextStepId;
		}
	}

	return Step.NextStepId;
}

FGameplayTag UMVInteractableComponent::ResolveStepTransition(
	const FMVInteractionSelectionStepData& Step,
	const FGameplayTag SelectedEntryId) const
{
	for (const FMVInteractionStepTransition& Transition : Step.Transitions)
	{
		if (Transition.TriggerId == SelectedEntryId)
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
	ActiveConfiguredMenuWindow->OnInteractionMenuEntrySelected.RemoveDynamic(
		this,
		&UMVInteractableComponent::HandleConfiguredMenuEntrySelected);
	ActiveConfiguredMenuWindow = nullptr;

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	const FGameplayTag NextStepId = bHasPendingStepAfterMenuClose
		? PendingStepAfterMenuClose
		: (Step ? Step->NextStepId : FGameplayTag());
	const bool bShouldExecutePendingCommands = bHasPendingCommandsAfterMenuClose;
	const TArray<FInstancedStruct> Commands = PendingCommandsAfterMenuClose;
	bHasPendingStepAfterMenuClose = false;
	bHasPendingCommandsAfterMenuClose = false;
	PendingStepAfterMenuClose = FGameplayTag();
	PendingCommandsAfterMenuClose.Reset();

	if (bShouldExecutePendingCommands)
	{
		ExecuteConfiguredCommands(Commands, NextStepId);
	}
	else
	{
		CompleteConfiguredStep(NextStepId);
	}
}

void UMVInteractableComponent::HandleConfiguredMenuEntrySelected(UObject* SourceObject, FMVMenuEntryData EntryData)
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

	OnInteractionMenuEntryRequested.Broadcast(ActiveInteractor.Get(), this, Step->StepId, EntryData);
	PendingStepAfterMenuClose = ResolveStepTransition(*SelectionStep, EntryData.EntryId);
	bHasPendingStepAfterMenuClose = true;
	PendingCommandsAfterMenuClose = EntryData.Commands;
	bHasPendingCommandsAfterMenuClose = !PendingCommandsAfterMenuClose.IsEmpty();

	if (ActiveConfiguredMenuWindow)
	{
		ActiveConfiguredMenuWindow->DeactivateWidgetWithFade();
	}
	else
	{
		const FGameplayTag NextStepId = PendingStepAfterMenuClose;
		const bool bShouldExecutePendingCommands = bHasPendingCommandsAfterMenuClose;
		const TArray<FInstancedStruct> Commands = PendingCommandsAfterMenuClose;
		bHasPendingStepAfterMenuClose = false;
		bHasPendingCommandsAfterMenuClose = false;
		PendingStepAfterMenuClose = FGameplayTag();
		PendingCommandsAfterMenuClose.Reset();

		if (bShouldExecutePendingCommands)
		{
			ExecuteConfiguredCommands(Commands, NextStepId);
		}
		else
		{
			CompleteConfiguredStep(NextStepId);
		}
	}
}

void UMVInteractableComponent::HandleConfiguredChoiceClosed(UMVInteractionChoiceWindow* ClosedChoiceWindow)
{
	if (!ClosedChoiceWindow || ClosedChoiceWindow != ActiveConfiguredChoiceWindow)
	{
		return;
	}

	ActiveConfiguredChoiceWindow->OnInteractionChoiceClosed.RemoveDynamic(
		this,
		&UMVInteractableComponent::HandleConfiguredChoiceClosed);
	ActiveConfiguredChoiceWindow->OnInteractionChoiceEntrySelected.RemoveDynamic(
		this,
		&UMVInteractableComponent::HandleConfiguredChoiceEntrySelected);
	ActiveConfiguredChoiceWindow = nullptr;

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	const FGameplayTag NextStepId = bHasPendingStepAfterMenuClose
		? PendingStepAfterMenuClose
		: (Step ? Step->NextStepId : FGameplayTag());
	const bool bShouldExecutePendingCommands = bHasPendingCommandsAfterMenuClose;
	const TArray<FInstancedStruct> Commands = PendingCommandsAfterMenuClose;
	bHasPendingStepAfterMenuClose = false;
	bHasPendingCommandsAfterMenuClose = false;
	PendingStepAfterMenuClose = FGameplayTag();
	PendingCommandsAfterMenuClose.Reset();

	if (bShouldExecutePendingCommands)
	{
		ExecuteConfiguredCommands(Commands, NextStepId);
	}
	else
	{
		CompleteConfiguredStep(NextStepId);
	}
}

void UMVInteractableComponent::HandleConfiguredChoiceEntrySelected(UObject* SourceObject, FMVMenuEntryData EntryData)
{
	if (SourceObject && SourceObject != this)
	{
		return;
	}

	const FInstancedStruct* StepInstance = FindInteractionStep(ActiveStepId);
	const FMVInteractionStepData* Step = StepInstance ? StepInstance->GetPtr<FMVInteractionStepData>() : nullptr;
	const FMVInteractionChoiceStepData* ChoiceStep =
		StepInstance ? StepInstance->GetPtr<FMVInteractionChoiceStepData>() : nullptr;
	if (!Step || !ChoiceStep)
	{
		return;
	}

	OnInteractionMenuEntryRequested.Broadcast(ActiveInteractor.Get(), this, Step->StepId, EntryData);
	PendingStepAfterMenuClose = ResolveChoiceTransition(*ChoiceStep, EntryData.EntryId);
	bHasPendingStepAfterMenuClose = true;
	PendingCommandsAfterMenuClose = EntryData.Commands;
	bHasPendingCommandsAfterMenuClose = !PendingCommandsAfterMenuClose.IsEmpty();

	if (ActiveConfiguredChoiceWindow)
	{
		ActiveConfiguredChoiceWindow->DeactivateWidgetWithFade();
	}
	else
	{
		const FGameplayTag NextStepId = PendingStepAfterMenuClose;
		const bool bShouldExecutePendingCommands = bHasPendingCommandsAfterMenuClose;
		const TArray<FInstancedStruct> Commands = PendingCommandsAfterMenuClose;
		bHasPendingStepAfterMenuClose = false;
		bHasPendingCommandsAfterMenuClose = false;
		PendingStepAfterMenuClose = FGameplayTag();
		PendingCommandsAfterMenuClose.Reset();

		if (bShouldExecutePendingCommands)
		{
			ExecuteConfiguredCommands(Commands, NextStepId);
		}
		else
		{
			CompleteConfiguredStep(NextStepId);
		}
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
