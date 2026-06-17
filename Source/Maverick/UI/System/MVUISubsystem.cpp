#include "UI/System/MVUISubsystem.h"

#include "CommonActivatableWidget.h"
#include "Components/MVStatComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Tables/MVDialogueTableTypes.h"
#include "Tables/MVTableManager.h"
#include "Tables/MVUIMessageTableTypes.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/Popup/MVInteractionPromptPopup.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/System/MVUISettings.h"
#include "UI/Window/MVDeathOverlayWindow.h"
#include "UI/Window/MVDialogueWindow.h"

void UMVUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMVUISubsystem::HandleWorldInit);
}

void UMVUISubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	ClearAllUI();

	Super::Deinitialize();
}

UMVUILayerBase* UMVUISubsystem::GetOrCreateRootLayer(UWorld* InWorld)
{
	for (int32 Index = LayerStack.Num() - 1; Index >= 0; --Index)
	{
		if (IsValid(LayerStack[Index]) && LayerStack[Index]->IsInViewport())
		{
			return LayerStack[Index];
		}
		LayerStack.RemoveAt(Index);
	}

	return PushLayer(InWorld);
}

UMVUILayerBase* UMVUISubsystem::PushLayer(UWorld* InWorld)
{
	UWorld* TargetWorld = InWorld ? InWorld : GetWorld();
	if (!TargetWorld || !TargetWorld->IsGameWorld())
	{
		return nullptr;
	}

	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	TSubclassOf<UMVUILayerBase> LayerClass = UMVUILayerBase::StaticClass();
	if (Settings && Settings->RootLayerClass)
	{
		LayerClass = Settings->RootLayerClass;
	}

	APlayerController* PlayerController = TargetWorld->GetFirstPlayerController();
	UMVUILayerBase* NewLayer = PlayerController
		? CreateWidget<UMVUILayerBase>(PlayerController, LayerClass)
		: CreateWidget<UMVUILayerBase>(TargetWorld, LayerClass);

	if (!NewLayer)
	{
		return nullptr;
	}

	const int32 BaseZOrder = Settings ? Settings->RootLayerZOrder : 0;
	const int32 ZOrderStep = Settings ? Settings->LayerZOrderStep : 100;
	NewLayer->AddToViewport(BaseZOrder + LayerStack.Num() * ZOrderStep);
	NewLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	LayerStack.Add(NewLayer);
	return NewLayer;
}

void UMVUISubsystem::PopLayer()
{
	if (LayerStack.IsEmpty())
	{
		return;
	}

	if (UMVUILayerBase* TopLayer = LayerStack.Pop())
	{
		TopLayer->ClearLayer();
		TopLayer->RemoveFromParent();
	}

	CachedHUD = nullptr;
	ActiveInteractionPrompt = nullptr;
	ActiveDialogueWindow = nullptr;
	ActivePopup = nullptr;
	bHasPendingDialogueRequest = false;
}

UCommonActivatableWidget* UMVUISubsystem::PushWindowByClass(TSubclassOf<UMVWindowBase> WindowClass)
{
	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer || !WindowClass)
	{
		return nullptr;
	}

	return Layer->PushWindow(WindowClass);
}

UMVPopupBase* UMVUISubsystem::PushPopupByClass(TSubclassOf<UMVPopupBase> PopupClass)
{
	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer || !PopupClass)
	{
		return nullptr;
	}

	CloseActivePopupImmediately();

	UMVPopupBase* NewPopup = Layer->PushPopup(PopupClass);
	TrackActivePopup(NewPopup);
	return NewPopup;
}

UMVHUDWidgetBase* UMVUISubsystem::ShowHUDByClass(TSubclassOf<UMVHUDWidgetBase> HUDClass)
{
	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer || !HUDClass)
	{
		return nullptr;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	CachedHUD = Cast<UMVHUDWidgetBase>(Layer->SetHUDByClass(HUDClass, PlayerController));
	if (CachedHUD)
	{
		CachedHUD->RefreshHUD();
	}

	return CachedHUD;
}

UMVHUDWidgetBase* UMVUISubsystem::ShowDefaultHUD()
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	return Settings ? ShowHUDByClass(Settings->DefaultHUDClass) : nullptr;
}

UCommonActivatableWidget* UMVUISubsystem::ShowLoadingWindow()
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	return Settings ? PushWindowByClass(Settings->LoadingWindowClass) : nullptr;
}

UCommonActivatableWidget* UMVUISubsystem::ShowDeathOverlay()
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings)
	{
		return nullptr;
	}

	UCommonActivatableWidget* DeathWindow = PushWindowByClass(Settings->DeathOverlayWindowClass);
	if (UMVDeathOverlayWindow* DeathOverlay = Cast<UMVDeathOverlayWindow>(DeathWindow))
	{
		DeathOverlay->SetDisplaySeconds(Settings->DeathOverlayDuration);
	}

	return DeathWindow;
}

UMVInteractionPromptPopup* UMVUISubsystem::ShowInteractionPrompt(const FMVInteractionPromptData& PromptData)
{
	if (IsDialogueWindowBlockingInteraction())
	{
		return nullptr;
	}

	if (IsPopupActive(ActiveInteractionPrompt) && !ActiveInteractionPrompt->IsClosing())
	{
		ActiveInteractionPrompt->SetPromptData(PromptData);
		return ActiveInteractionPrompt;
	}

	if (IsPopupActive(ActivePopup))
	{
		return nullptr;
	}

	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings)
	{
		return nullptr;
	}

	ActiveInteractionPrompt = Cast<UMVInteractionPromptPopup>(PushPopupByClass(Settings->InteractionPromptPopupClass));
	if (ActiveInteractionPrompt)
	{
		ActiveInteractionPrompt->SetPromptData(PromptData);
	}

	return ActiveInteractionPrompt;
}

UMVInteractionPromptPopup* UMVUISubsystem::ShowInteractionPromptText(FText PromptText)
{
	FMVInteractionPromptData PromptData;
	PromptData.PromptText = PromptText;
	return ShowInteractionPrompt(PromptData);
}

void UMVUISubsystem::HideInteractionPrompt()
{
	if (IsPopupActive(ActiveInteractionPrompt))
	{
		ActiveInteractionPrompt->ClosePopup();
	}
}

UMVDialogueWindow* UMVUISubsystem::ShowDialogueWindowText(FText DialogueText, float Duration)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->DialogueWindowClass)
	{
		return nullptr;
	}

	if (IsPopupActive(ActivePopup))
	{
		QueueDialogueWindowText(DialogueText, Duration);
		CloseActivePopup();
		return nullptr;
	}

	return OpenDialogueWindowText(DialogueText, Duration);
}

UMVDialogueWindow* UMVUISubsystem::OpenDialogueWindowText(FText DialogueText, float Duration)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->DialogueWindowClass)
	{
		return nullptr;
	}

	const float DisplayDuration = Duration >= 0.0f ? Duration : Settings->DialogueWindowDuration;
	if (IsDialogueWindowActive(ActiveDialogueWindow))
	{
		ActiveDialogueWindow->SetAutoDismissSeconds(DisplayDuration);
		ActiveDialogueWindow->SetDialogueText(DialogueText);
		return ActiveDialogueWindow;
	}

	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer)
	{
		return nullptr;
	}

	ActiveDialogueWindow = Cast<UMVDialogueWindow>(Layer->PushWindow(Settings->DialogueWindowClass));
	if (ActiveDialogueWindow)
	{
		TrackActiveDialogueWindow(ActiveDialogueWindow);
		ActiveDialogueWindow->SetAutoDismissSeconds(DisplayDuration);
		ActiveDialogueWindow->SetDialogueText(DialogueText);
	}

	bHasPendingDialogueRequest = false;
	return ActiveDialogueWindow;
}

void UMVUISubsystem::HideDialogueWindow()
{
	bHasPendingDialogueRequest = false;

	if (IsDialogueWindowActive(ActiveDialogueWindow))
	{
		ActiveDialogueWindow->CloseDialogue();
		return;
	}

	ActiveDialogueWindow = nullptr;
}

void UMVUISubsystem::SkipDialogueWindow()
{
	HideDialogueWindow();
}

bool UMVUISubsystem::IsDialogueWindowActive() const
{
	return IsDialogueWindowActive(ActiveDialogueWindow);
}

bool UMVUISubsystem::IsDialogueWindowBlockingInteraction() const
{
	return bHasPendingDialogueRequest || IsDialogueWindowActive(ActiveDialogueWindow);
}

UMVDialogueWindow* UMVUISubsystem::ShowDialogueWindowById(FName DialogueId)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || Settings->DialogueTableName.IsNone() || DialogueId.IsNone())
	{
		return nullptr;
	}

	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		return nullptr;
	}

	const FMVDialogueRow* DialogueRow = TableManager->FindRow<FMVDialogueRow>(
		Settings->DialogueTableName,
		DialogueId.ToString());
	if (!DialogueRow)
	{
		return nullptr;
	}

	return ShowDialogueWindowText(DialogueRow->DialogueText, DialogueRow->DisplayDuration);
}

UMVMessagePopup* UMVUISubsystem::ShowPopupMessage(const FMVPopupMessageData& MessageData)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->MessagePopupClass)
	{
		return nullptr;
	}

	UMVMessagePopup* MessagePopup = Cast<UMVMessagePopup>(PushPopupByClass(Settings->MessagePopupClass));
	if (MessagePopup)
	{
		MessagePopup->SetMessageData(MessageData);
	}

	return MessagePopup;
}

UMVMessagePopup* UMVUISubsystem::ShowPopupMessageText(FText MessageText, float Duration)
{
	FMVPopupMessageData MessageData;
	MessageData.MessageText = MessageText;
	MessageData.Duration = Duration;
	return ShowPopupMessage(MessageData);
}

UMVMessagePopup* UMVUISubsystem::ShowPopupMessageById(FName MessageId)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || Settings->UIMessageTableName.IsNone() || MessageId.IsNone())
	{
		return nullptr;
	}

	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		return nullptr;
	}

	const FMVUIMessageRow* MessageRow = TableManager->FindRow<FMVUIMessageRow>(
		Settings->UIMessageTableName,
		MessageId.ToString());
	if (!MessageRow)
	{
		return nullptr;
	}

	FMVPopupMessageData MessageData;
	MessageData.MessageId = MessageId;
	MessageData.MessageText = MessageRow->MessageText;
	MessageData.Duration = MessageRow->DefaultDuration;
	return ShowPopupMessage(MessageData);
}

void UMVUISubsystem::ClearAllUI()
{
	for (UMVUILayerBase* Layer : LayerStack)
	{
		if (!Layer)
		{
			continue;
		}

		Layer->ClearLayer();
		Layer->RemoveFromParent();
	}

	LayerStack.Reset();
	CachedHUD = nullptr;
	ActiveInteractionPrompt = nullptr;
	ActiveDialogueWindow = nullptr;
	ActivePopup = nullptr;
	bHasPendingDialogueRequest = false;
}

void UMVUISubsystem::HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	ClearAllUI();
	GetOrCreateRootLayer(World);

	FTimerHandle PostWorldInitTimerHandle;
	const TWeakObjectPtr<UWorld> WeakWorld = World;
	World->GetTimerManager().SetTimer(
		PostWorldInitTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
		{
			UWorld* CurrentWorld = WeakWorld.Get();
			if (!CurrentWorld || !CurrentWorld->IsGameWorld())
			{
				return;
			}

			ShowDefaultHUD();
			BindToPlayerDeath(CurrentWorld);
		}),
		0.1f,
		false);
}

void UMVUISubsystem::BindToPlayerDeath(UWorld* World)
{
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	UMVStatComponent* StatComponent = Pawn ? Pawn->FindComponentByClass<UMVStatComponent>() : nullptr;
	if (!StatComponent)
	{
		return;
	}

	StatComponent->OnDead.RemoveDynamic(this, &UMVUISubsystem::HandlePlayerDeath);
	StatComponent->OnDead.AddDynamic(this, &UMVUISubsystem::HandlePlayerDeath);
}

void UMVUISubsystem::HandlePlayerDeath()
{
	ShowDeathOverlay();
}

void UMVUISubsystem::HandlePopupClosed(UMVPopupBase* ClosedPopup)
{
	if (!ClosedPopup)
	{
		return;
	}

	ClosedPopup->OnPopupClosed.RemoveDynamic(this, &UMVUISubsystem::HandlePopupClosed);

	if (ClosedPopup == ActiveInteractionPrompt)
	{
		ActiveInteractionPrompt = nullptr;
	}

	if (ClosedPopup == ActivePopup)
	{
		ActivePopup = nullptr;
	}

	TryOpenPendingDialogueWindow();
}

void UMVUISubsystem::HandleDialogueWindowClosed(UMVDialogueWindow* ClosedDialogueWindow)
{
	if (!ClosedDialogueWindow)
	{
		return;
	}

	ClosedDialogueWindow->OnDialogueWindowClosed.RemoveDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosed);

	if (ClosedDialogueWindow == ActiveDialogueWindow)
	{
		ActiveDialogueWindow = nullptr;
	}
}

bool UMVUISubsystem::IsPopupActive(const UMVPopupBase* Popup) const
{
	return IsValid(Popup) && Popup->GetParent();
}

bool UMVUISubsystem::IsDialogueWindowActive(const UMVDialogueWindow* DialogueWindow) const
{
	return IsValid(DialogueWindow) && DialogueWindow->IsActivated();
}

void UMVUISubsystem::CloseActivePopupImmediately()
{
	if (!IsPopupActive(ActivePopup))
	{
		ActivePopup = nullptr;
		return;
	}

	ActivePopup->ClosePopupImmediately();
}

void UMVUISubsystem::CloseActivePopup()
{
	if (!IsPopupActive(ActivePopup))
	{
		ActivePopup = nullptr;
		return;
	}

	ActivePopup->ClosePopup();
}

void UMVUISubsystem::QueueDialogueWindowText(FText DialogueText, float Duration)
{
	PendingDialogueText = DialogueText;
	PendingDialogueDuration = Duration;
	bHasPendingDialogueRequest = true;
}

void UMVUISubsystem::TryOpenPendingDialogueWindow()
{
	if (!bHasPendingDialogueRequest || IsPopupActive(ActivePopup))
	{
		return;
	}

	const FText DialogueText = PendingDialogueText;
	const float DialogueDuration = PendingDialogueDuration;
	bHasPendingDialogueRequest = false;
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;

	OpenDialogueWindowText(DialogueText, DialogueDuration);
}

void UMVUISubsystem::TrackActivePopup(UMVPopupBase* Popup)
{
	if (IsValid(ActivePopup))
	{
		ActivePopup->OnPopupClosed.RemoveDynamic(this, &UMVUISubsystem::HandlePopupClosed);
	}

	ActivePopup = Popup;
	if (IsValid(ActivePopup))
	{
		ActivePopup->OnPopupClosed.AddUniqueDynamic(this, &UMVUISubsystem::HandlePopupClosed);
	}
}

void UMVUISubsystem::TrackActiveDialogueWindow(UMVDialogueWindow* DialogueWindow)
{
	if (IsValid(ActiveDialogueWindow))
	{
		ActiveDialogueWindow->OnDialogueWindowClosed.RemoveDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosed);
	}

	ActiveDialogueWindow = DialogueWindow;
	if (IsValid(ActiveDialogueWindow))
	{
		ActiveDialogueWindow->OnDialogueWindowClosed.AddUniqueDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosed);
	}
}
