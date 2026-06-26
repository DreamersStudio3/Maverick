#include "UI/System/MVUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Character/MVCharacterBase.h"
#include "CommonActivatableWidget.h"
#include "Components/MVStatComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Tables/MVDialogueTableTypes.h"
#include "Tables/MVTableManager.h"
#include "Tables/MVUIMessageTableTypes.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/Popup/MVInteractionPromptPopup.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/Debug/MVPIEActionTestWidget.h"
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
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;
	PendingDialogueMinimumSkipDelay = -1.0f;
	bDialoguePromptRestoreDelayActive = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DialoguePromptRestoreDelayTimerHandle);
		World->GetTimerManager().ClearTimer(DialogueCameraZoomTimerHandle);
	}
	if (USpringArmComponent* SpringArm = DialogueZoomSpringArm.Get())
	{
		SpringArm->TargetArmLength = DialogueZoomOriginalArmLength;
	}
	if (UCameraComponent* Camera = DialogueZoomCamera.Get())
	{
		Camera->SetFieldOfView(DialogueZoomOriginalFOV);
	}
	DialogueZoomSpringArm.Reset();
	DialogueZoomCamera.Reset();
	bDialogueCameraZoomApplied = false;
	bDialogueCameraZoomRestoring = false;
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
	if (IsDialogueWindowBlockingInteraction() || IsPIEActionTestPanelActiveOrPending())
	{
		HideInteractionPrompt();
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
	return ShowDialogueWindowTextWithTiming(DialogueText, Duration);
}

UMVDialogueWindow* UMVUISubsystem::ShowDialogueWindowTextWithTiming(FText DialogueText, float Duration, float MinimumSkipDelay)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->DialogueWindowClass)
	{
		return nullptr;
	}

	if (IsDialogueWindowBlockingInteraction())
	{
		return ActiveDialogueWindow;
	}

	if (IsPopupActive(ActivePopup))
	{
		QueueDialogueWindowText(DialogueText, Duration, MinimumSkipDelay);
		CloseActivePopup();
		return nullptr;
	}

	return OpenDialogueWindowText(DialogueText, Duration, MinimumSkipDelay);
}

UMVPIEActionTestWidget* UMVUISubsystem::ShowPIEActionTestPanel(AMVCharacterBase* TargetCharacter)
{
#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	if (!World || World->WorldType != EWorldType::PIE)
	{
		return nullptr;
	}

	AMVCharacterBase* ResolvedTargetCharacter = ResolvePIEActionTestTargetCharacter(TargetCharacter);
	if (!ResolvedTargetCharacter)
	{
		return nullptr;
	}

	if (IsDialogueWindowBlockingInteraction())
	{
		bHasPendingPIEActionTestPanel = true;
		PendingPIEActionTestTargetCharacter = ResolvedTargetCharacter;
		return ActivePIEActionTestWidget;
	}

	return OpenPIEActionTestPanel(ResolvedTargetCharacter);
#else
	return nullptr;
#endif
}

UMVPIEActionTestWidget* UMVUISubsystem::OpenPIEActionTestPanel(AMVCharacterBase* TargetCharacter)
{
#if !UE_BUILD_SHIPPING
	if (!TargetCharacter)
	{
		return nullptr;
	}

	APlayerController* PlayerController = ResolvePIEActionTestPlayerController(TargetCharacter);
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return nullptr;
	}

	if (ActivePIEActionTestWidget && ActivePIEActionTestWidget->IsInViewport())
	{
		ActivePIEActionTestWidget->SetTargetCharacter(TargetCharacter);
		return ActivePIEActionTestWidget;
	}

	ActivePIEActionTestWidget = CreateWidget<UMVPIEActionTestWidget>(
		PlayerController,
		UMVPIEActionTestWidget::StaticClass());
	if (!ActivePIEActionTestWidget)
	{
		return nullptr;
	}

	ActivePIEActionTestWidget->SetTargetCharacter(TargetCharacter);
	ActivePIEActionTestWidget->AddToViewport(9000);
	ActivePIEActionTestWidget->SetKeyboardFocus();

	PlayerController->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ActivePIEActionTestWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
	return ActivePIEActionTestWidget;
#else
	return nullptr;
#endif
}

void UMVUISubsystem::HidePIEActionTestPanel()
{
#if !UE_BUILD_SHIPPING
	bHasPendingPIEActionTestPanel = false;
	PendingPIEActionTestTargetCharacter.Reset();
	if (ActivePIEActionTestWidget)
	{
		ActivePIEActionTestWidget->RemoveFromParent();
		ActivePIEActionTestWidget = nullptr;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
#endif
}

bool UMVUISubsystem::IsPIEActionTestPanelActiveOrPending() const
{
#if !UE_BUILD_SHIPPING
	return bHasPendingPIEActionTestPanel
		|| (ActivePIEActionTestWidget && ActivePIEActionTestWidget->IsInViewport());
#else
	return false;
#endif
}

UMVDialogueWindow* UMVUISubsystem::OpenDialogueWindowText(FText DialogueText, float Duration, float MinimumSkipDelay)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->DialogueWindowClass)
	{
		return nullptr;
	}

	const float DisplayDuration = Duration >= 0.0f ? Duration : Settings->DialogueWindowDuration;
	const float ResolvedMinimumSkipDelay = MinimumSkipDelay >= 0.0f
		? MinimumSkipDelay
		: Settings->DialogueWindowMinimumSkipDelay;
	if (IsDialogueWindowPresent(ActiveDialogueWindow))
	{
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
		ApplyDialogueCameraZoom(ActiveDialogueWindow->GetFadeInSeconds());
		ActiveDialogueWindow->SetMinimumSkipDelay(ResolvedMinimumSkipDelay);
		ActiveDialogueWindow->SetAutoDismissSeconds(DisplayDuration);
		ActiveDialogueWindow->SetDialogueText(DialogueText);
	}

	bHasPendingDialogueRequest = false;
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;
	PendingDialogueMinimumSkipDelay = -1.0f;
	return ActiveDialogueWindow;
}

void UMVUISubsystem::HideDialogueWindow()
{
	bHasPendingDialogueRequest = false;
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;
	PendingDialogueMinimumSkipDelay = -1.0f;

	if (IsDialogueWindowPresent(ActiveDialogueWindow))
	{
		ActiveDialogueWindow->CloseDialogue();
		return;
	}

	ActiveDialogueWindow = nullptr;
}

void UMVUISubsystem::SkipDialogueWindow()
{
	if (CanSkipDialogueWindow())
	{
		HideDialogueWindow();
	}
}

bool UMVUISubsystem::CanSkipDialogueWindow() const
{
	return IsDialogueWindowPresent(ActiveDialogueWindow)
		&& ActiveDialogueWindow->CanSkipDialogue();
}

bool UMVUISubsystem::CanUseInteractionPrompt() const
{
	return !IsDialogueWindowBlockingInteraction()
		&& !IsPIEActionTestPanelActiveOrPending()
		&& IsPopupActive(ActiveInteractionPrompt)
		&& ActivePopup == ActiveInteractionPrompt
		&& !ActiveInteractionPrompt->IsClosing()
		&& !ActiveInteractionPrompt->IsFading();
}

bool UMVUISubsystem::IsDialogueWindowActive() const
{
	return IsDialogueWindowActive(ActiveDialogueWindow);
}

bool UMVUISubsystem::IsDialogueWindowBlockingInteraction() const
{
	return bHasPendingDialogueRequest
		|| bDialoguePromptRestoreDelayActive
		|| IsDialogueWindowPresent(ActiveDialogueWindow);
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

	return ShowDialogueWindowTextWithTiming(
		DialogueRow->DialogueText,
		DialogueRow->DisplayDuration,
		DialogueRow->MinimumSkipDelay);
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
	if (ActivePIEActionTestWidget)
	{
		ActivePIEActionTestWidget->RemoveFromParent();
	}
	ActivePIEActionTestWidget = nullptr;
	PendingPIEActionTestTargetCharacter.Reset();
	bHasPendingPIEActionTestPanel = false;
	bHasPendingDialogueRequest = false;
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;
	PendingDialogueMinimumSkipDelay = -1.0f;
	bDialoguePromptRestoreDelayActive = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DialoguePromptRestoreDelayTimerHandle);
		World->GetTimerManager().ClearTimer(DialogueCameraZoomTimerHandle);
	}
	if (USpringArmComponent* SpringArm = DialogueZoomSpringArm.Get())
	{
		SpringArm->TargetArmLength = DialogueZoomOriginalArmLength;
	}
	if (UCameraComponent* Camera = DialogueZoomCamera.Get())
	{
		Camera->SetFieldOfView(DialogueZoomOriginalFOV);
	}
	DialogueZoomSpringArm.Reset();
	DialogueZoomCamera.Reset();
	bDialogueCameraZoomApplied = false;
	bDialogueCameraZoomRestoring = false;
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

AMVCharacterBase* UMVUISubsystem::ResolvePIEActionTestTargetCharacter(AMVCharacterBase* TargetCharacter) const
{
	if (TargetCharacter)
	{
		return TargetCharacter;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	return PlayerController
		? Cast<AMVCharacterBase>(PlayerController->GetPawn())
		: nullptr;
}

APlayerController* UMVUISubsystem::ResolvePIEActionTestPlayerController(const AMVCharacterBase* TargetCharacter) const
{
	if (!TargetCharacter)
	{
		return nullptr;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(TargetCharacter->GetController()))
	{
		return PlayerController;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	return PlayerController;
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
	ClosedDialogueWindow->OnDialogueWindowClosing.RemoveDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosing);

	if (ClosedDialogueWindow == ActiveDialogueWindow)
	{
		ActiveDialogueWindow = nullptr;
	}

	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	const float RestoreDelay = Settings ? Settings->DialoguePromptRestoreDelay : 0.0f;
	if (RestoreDelay > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			bDialoguePromptRestoreDelayActive = true;
			World->GetTimerManager().SetTimer(
				DialoguePromptRestoreDelayTimerHandle,
				this,
				&UMVUISubsystem::HandleDialoguePromptRestoreDelayElapsed,
				RestoreDelay,
				false);
		}
		else
		{
			bDialoguePromptRestoreDelayActive = false;
		}
	}
	else
	{
		bDialoguePromptRestoreDelayActive = false;
	}

	if (bHasPendingPIEActionTestPanel)
	{
		AMVCharacterBase* PendingTargetCharacter = PendingPIEActionTestTargetCharacter.Get();
		PendingPIEActionTestTargetCharacter.Reset();
		bHasPendingPIEActionTestPanel = false;
		if (PendingTargetCharacter)
		{
			OpenPIEActionTestPanel(PendingTargetCharacter);
		}
	}
}

void UMVUISubsystem::HandleDialogueWindowClosing(UMVDialogueWindow* ClosingDialogueWindow)
{
	const float DurationSeconds = IsValid(ClosingDialogueWindow)
		? ClosingDialogueWindow->GetFadeOutSeconds()
		: -1.0f;
	RestoreDialogueCameraZoom(DurationSeconds);
}

void UMVUISubsystem::HandleDialoguePromptRestoreDelayElapsed()
{
	bDialoguePromptRestoreDelayActive = false;
}

void UMVUISubsystem::ApplyDialogueCameraZoom(float DurationOverride)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->bEnableDialogueCameraZoom || bDialogueCameraZoomApplied)
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	DialogueZoomSpringArm = Pawn->FindComponentByClass<USpringArmComponent>();
	DialogueZoomCamera = Pawn->FindComponentByClass<UCameraComponent>();
	if (!DialogueZoomSpringArm.IsValid() && !DialogueZoomCamera.IsValid())
	{
		return;
	}

	if (USpringArmComponent* SpringArm = DialogueZoomSpringArm.Get())
	{
		DialogueZoomOriginalArmLength = SpringArm->TargetArmLength;
		DialogueZoomTargetArmLength = FMath::Max(0.0f, DialogueZoomOriginalArmLength + Settings->DialogueCameraSpringArmLengthOffset);
	}
	if (UCameraComponent* Camera = DialogueZoomCamera.Get())
	{
		DialogueZoomOriginalFOV = Camera->FieldOfView;
		DialogueZoomTargetFOV = FMath::Clamp(DialogueZoomOriginalFOV + Settings->DialogueCameraFOVOffset, 5.0f, 170.0f);
	}

	bDialogueCameraZoomApplied = true;
	StartDialogueCameraZoom(false, DurationOverride);
}

void UMVUISubsystem::RestoreDialogueCameraZoom(float DurationOverride)
{
	if (!bDialogueCameraZoomApplied || bDialogueCameraZoomRestoring)
	{
		return;
	}

	StartDialogueCameraZoom(true, DurationOverride);
}

void UMVUISubsystem::StartDialogueCameraZoom(bool bInRestoring, float DurationOverride)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	UWorld* World = GetWorld();
	if (!Settings || !World)
	{
		FinishDialogueCameraZoom();
		return;
	}

	bDialogueCameraZoomRestoring = bInRestoring;
	DialogueZoomDurationSeconds = ResolveDialogueCameraZoomDuration(DurationOverride, bInRestoring);
	DialogueZoomDecelerationExponent = FMath::Max(1.0f, Settings->DialogueCameraZoomDecelerationExponent);
	DialogueZoomStartTimeSeconds = World->GetTimeSeconds();

	if (USpringArmComponent* SpringArm = DialogueZoomSpringArm.Get())
	{
		DialogueZoomStartArmLength = SpringArm->TargetArmLength;
		DialogueZoomTargetArmLength = bInRestoring
			? DialogueZoomOriginalArmLength
			: FMath::Max(0.0f, DialogueZoomOriginalArmLength + Settings->DialogueCameraSpringArmLengthOffset);
	}
	if (UCameraComponent* Camera = DialogueZoomCamera.Get())
	{
		DialogueZoomStartFOV = Camera->FieldOfView;
		DialogueZoomTargetFOV = bInRestoring
			? DialogueZoomOriginalFOV
			: FMath::Clamp(DialogueZoomOriginalFOV + Settings->DialogueCameraFOVOffset, 5.0f, 170.0f);
	}

	World->GetTimerManager().ClearTimer(DialogueCameraZoomTimerHandle);
	if (DialogueZoomDurationSeconds <= 0.0f)
	{
		UpdateDialogueCameraZoom();
		FinishDialogueCameraZoom();
		return;
	}

	World->GetTimerManager().SetTimer(
		DialogueCameraZoomTimerHandle,
		this,
		&UMVUISubsystem::UpdateDialogueCameraZoom,
		0.01f,
		true);
}

float UMVUISubsystem::ResolveDialogueCameraZoomDuration(float DurationOverride, bool bRestoring) const
{
	if (DurationOverride >= 0.0f)
	{
		return DurationOverride;
	}

	if (IsValid(ActiveDialogueWindow))
	{
		return bRestoring
			? ActiveDialogueWindow->GetFadeOutSeconds()
			: ActiveDialogueWindow->GetFadeInSeconds();
	}

	return 0.0f;
}

void UMVUISubsystem::UpdateDialogueCameraZoom()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FinishDialogueCameraZoom();
		return;
	}

	const float LinearAlpha = DialogueZoomDurationSeconds > 0.0f
		? FMath::Clamp((World->GetTimeSeconds() - DialogueZoomStartTimeSeconds) / DialogueZoomDurationSeconds, 0.0f, 1.0f)
		: 1.0f;
	const float Alpha = 1.0f - FMath::Pow(1.0f - LinearAlpha, DialogueZoomDecelerationExponent);

	if (USpringArmComponent* SpringArm = DialogueZoomSpringArm.Get())
	{
		SpringArm->TargetArmLength = FMath::Lerp(DialogueZoomStartArmLength, DialogueZoomTargetArmLength, Alpha);
	}
	if (UCameraComponent* Camera = DialogueZoomCamera.Get())
	{
		Camera->SetFieldOfView(FMath::Lerp(DialogueZoomStartFOV, DialogueZoomTargetFOV, Alpha));
	}

	if (LinearAlpha >= 1.0f)
	{
		FinishDialogueCameraZoom();
	}
}

void UMVUISubsystem::FinishDialogueCameraZoom()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DialogueCameraZoomTimerHandle);
	}

	if (USpringArmComponent* SpringArm = DialogueZoomSpringArm.Get())
	{
		SpringArm->TargetArmLength = DialogueZoomTargetArmLength;
	}
	if (UCameraComponent* Camera = DialogueZoomCamera.Get())
	{
		Camera->SetFieldOfView(DialogueZoomTargetFOV);
	}

	if (bDialogueCameraZoomRestoring)
	{
		DialogueZoomSpringArm.Reset();
		DialogueZoomCamera.Reset();
		bDialogueCameraZoomApplied = false;
		bDialogueCameraZoomRestoring = false;
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

bool UMVUISubsystem::IsDialogueWindowPresent(const UMVDialogueWindow* DialogueWindow) const
{
	return IsValid(DialogueWindow);
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

void UMVUISubsystem::QueueDialogueWindowText(FText DialogueText, float Duration, float MinimumSkipDelay)
{
	PendingDialogueText = DialogueText;
	PendingDialogueDuration = Duration;
	PendingDialogueMinimumSkipDelay = MinimumSkipDelay;
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
	const float DialogueMinimumSkipDelay = PendingDialogueMinimumSkipDelay;
	bHasPendingDialogueRequest = false;
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;
	PendingDialogueMinimumSkipDelay = -1.0f;

	OpenDialogueWindowText(DialogueText, DialogueDuration, DialogueMinimumSkipDelay);
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
		ActiveDialogueWindow->OnDialogueWindowClosing.RemoveDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosing);
		ActiveDialogueWindow->OnDialogueWindowClosed.RemoveDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosed);
	}

	ActiveDialogueWindow = DialogueWindow;
	if (IsValid(ActiveDialogueWindow))
	{
		ActiveDialogueWindow->OnDialogueWindowClosing.AddUniqueDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosing);
		ActiveDialogueWindow->OnDialogueWindowClosed.AddUniqueDynamic(this, &UMVUISubsystem::HandleDialogueWindowClosed);
	}
}
