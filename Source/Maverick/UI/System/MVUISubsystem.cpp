#include "UI/System/MVUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "CommonActivatableWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HAL/IConsoleManager.h"
#include "Tables/MVDialogueTableTypes.h"
#include "Tables/MVTableManager.h"
#include "Tables/MVUIMessageTableTypes.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/Popup/MVDialoguePopup.h"
#include "UI/Popup/MVInteractionChoicePopup.h"
#include "UI/Popup/MVInteractionPromptPopup.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/System/MVUISettings.h"
#include "UI/Window/MVDeathOverlayWindow.h"
#include "UI/Window/MVInteractionMenuWindow.h"
#include "UI/Window/MVLoadingWindow.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVUISubsystem, Log, All);

namespace
{
UMVUISubsystem* MVUISubsystemResolveLoadingTestSubsystem(UWorld* World)
{
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
}

bool MVUISubsystemShouldUseNativeLoadingTestWindow(const TArray<FString>& Args)
{
	for (const FString& Arg : Args)
	{
		if (Arg.Equals(TEXT("native"), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

void MVUISubsystemShowLoadingTestCommand(const TArray<FString>& Args, UWorld* World)
{
	if (UMVUISubsystem* UISubsystem = MVUISubsystemResolveLoadingTestSubsystem(World))
	{
		UISubsystem->ShowLoadingWindowForTest(MVUISubsystemShouldUseNativeLoadingTestWindow(Args));
	}
}

void MVUISubsystemHideLoadingTestCommand(const TArray<FString>& Args, UWorld* World)
{
	if (UMVUISubsystem* UISubsystem = MVUISubsystemResolveLoadingTestSubsystem(World))
	{
		UISubsystem->HideLoadingWindowForTest();
	}
}

void MVUISubsystemAdvanceLoadingTestCardCommand(const TArray<FString>& Args, UWorld* World)
{
	if (UMVUISubsystem* UISubsystem = MVUISubsystemResolveLoadingTestSubsystem(World))
	{
		UISubsystem->AdvanceLoadingGuideCardForTest();
	}
}

#if !UE_BUILD_SHIPPING
static FAutoConsoleCommandWithWorldAndArgs MVUISubsystemShowLoadingTestConsoleCommand(
	TEXT("MV.UI.LoadingTest.Show"),
	TEXT("Show a persistent loading window for UI testing. Pass 'native' to use the C++ fallback layout."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&MVUISubsystemShowLoadingTestCommand));

static FAutoConsoleCommandWithWorldAndArgs MVUISubsystemHideLoadingTestConsoleCommand(
	TEXT("MV.UI.LoadingTest.Hide"),
	TEXT("Hide the persistent loading window test and restore the default UI."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&MVUISubsystemHideLoadingTestCommand));

static FAutoConsoleCommandWithWorldAndArgs MVUISubsystemAdvanceLoadingTestCardConsoleCommand(
	TEXT("MV.UI.LoadingTest.Advance"),
	TEXT("Advance the guide card in the active loading window test."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&MVUISubsystemAdvanceLoadingTestCardCommand));

#endif
}

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
	ActiveInteractionMenuWindow = nullptr;
	ActiveInteractionChoicePopup = nullptr;
	ActiveDialoguePopup = nullptr;
	ActivePopup = nullptr;
	ActiveLoadingWindowForTest = nullptr;
	ActiveInteractionMenuSource.Reset();
	ActiveInteractionChoiceSource.Reset();
	InteractionSessionSources.Reset();
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

void UMVUISubsystem::HideHUD()
{
	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (Layer)
	{
		Layer->HideHUDWidgetWithFade();
	}

	CachedHUD = nullptr;
}

UCommonActivatableWidget* UMVUISubsystem::ShowLoadingWindow()
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->LoadingWindowClass)
	{
		return nullptr;
	}

	HideHUD();
	return PushWindowByClass(Settings->LoadingWindowClass);
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
	if (IsDialoguePopupBlockingInteraction()
		|| IsInteractionMenuActive()
		|| IsInteractionChoiceActive()
		|| IsInteractionSessionActive())
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

UMVInteractionMenuWindow* UMVUISubsystem::ShowInteractionMenu(const FMVInteractionMenuData& MenuData, UObject* SourceObject)
{
	HideInteractionPrompt();

	if (ActiveInteractionMenuWindow && ActiveInteractionMenuWindow->IsActivated())
	{
		ActiveInteractionMenuWindow->SetMenuData(MenuData, SourceObject);
		ActiveInteractionMenuSource = SourceObject ? SourceObject : ActiveInteractionMenuWindow;
		BeginInteractionSession(ActiveInteractionMenuSource.Get());
		return ActiveInteractionMenuWindow;
	}

	ActiveInteractionMenuWindow = Cast<UMVInteractionMenuWindow>(PushWindowByClass(UMVInteractionMenuWindow::StaticClass()));
	if (!ActiveInteractionMenuWindow)
	{
		return nullptr;
	}

	ActiveInteractionMenuSource = SourceObject ? SourceObject : ActiveInteractionMenuWindow;
	BeginInteractionSession(ActiveInteractionMenuSource.Get());
	ActiveInteractionMenuWindow->OnInteractionMenuClosed.AddUniqueDynamic(this, &UMVUISubsystem::HandleInteractionMenuClosed);
	ActiveInteractionMenuWindow->SetMenuData(MenuData, SourceObject);
	return ActiveInteractionMenuWindow;
}

void UMVUISubsystem::HideInteractionMenu()
{
	if (ActiveInteractionMenuWindow && ActiveInteractionMenuWindow->IsActivated())
	{
		ActiveInteractionMenuWindow->DeactivateWidgetWithFade();
	}
}

bool UMVUISubsystem::IsInteractionMenuActive() const
{
	return ActiveInteractionMenuWindow && ActiveInteractionMenuWindow->IsActivated();
}

UMVInteractionChoicePopup* UMVUISubsystem::ShowInteractionChoice(
	const FMVInteractionChoiceData& ChoiceData,
	UObject* SourceObject)
{
	HideInteractionPrompt();

	if (IsPopupActive(ActiveInteractionChoicePopup) && !ActiveInteractionChoicePopup->IsClosing())
	{
		ActiveInteractionChoicePopup->SetChoiceData(ChoiceData, SourceObject);
		ActiveInteractionChoiceSource = SourceObject ? SourceObject : ActiveInteractionChoicePopup;
		BeginInteractionSession(ActiveInteractionChoiceSource.Get());
		return ActiveInteractionChoicePopup;
	}

	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	ActiveInteractionChoicePopup = Layer
		? Cast<UMVInteractionChoicePopup>(Layer->PushPopup(UMVInteractionChoicePopup::StaticClass()))
		: nullptr;
	if (!ActiveInteractionChoicePopup)
	{
		return nullptr;
	}

	ActiveInteractionChoiceSource = SourceObject ? SourceObject : ActiveInteractionChoicePopup;
	BeginInteractionSession(ActiveInteractionChoiceSource.Get());
	ActiveInteractionChoicePopup->OnPopupClosed.AddUniqueDynamic(
		this,
		&UMVUISubsystem::HandleInteractionChoiceClosed);
	ActiveInteractionChoicePopup->SetChoiceData(ChoiceData, SourceObject);
	return ActiveInteractionChoicePopup;
}

void UMVUISubsystem::HideInteractionChoice()
{
	if (IsPopupActive(ActiveInteractionChoicePopup) && !ActiveInteractionChoicePopup->IsClosing())
	{
		ActiveInteractionChoicePopup->ClosePopup();
	}
}

bool UMVUISubsystem::IsInteractionChoiceActive() const
{
	return IsPopupActive(ActiveInteractionChoicePopup) && !ActiveInteractionChoicePopup->IsClosing();
}

void UMVUISubsystem::BeginInteractionSession(UObject* SourceObject)
{
	UObject* SessionSource = SourceObject ? SourceObject : this;
	if (!SessionSource)
	{
		return;
	}

	for (int32 Index = InteractionSessionSources.Num() - 1; Index >= 0; --Index)
	{
		UObject* ExistingSource = InteractionSessionSources[Index].Get();
		if (!ExistingSource)
		{
			InteractionSessionSources.RemoveAt(Index);
			continue;
		}

		if (ExistingSource == SessionSource)
		{
			HideInteractionPrompt();
			return;
		}
	}

	InteractionSessionSources.Add(SessionSource);
	HideInteractionPrompt();
}

void UMVUISubsystem::EndInteractionSession(UObject* SourceObject)
{
	UObject* SessionSource = SourceObject ? SourceObject : this;
	for (int32 Index = InteractionSessionSources.Num() - 1; Index >= 0; --Index)
	{
		UObject* ExistingSource = InteractionSessionSources[Index].Get();
		if (!ExistingSource || ExistingSource == SessionSource)
		{
			InteractionSessionSources.RemoveAt(Index);
		}
	}
}

bool UMVUISubsystem::IsInteractionSessionActive() const
{
	for (const TWeakObjectPtr<UObject>& SessionSource : InteractionSessionSources)
	{
		if (SessionSource.IsValid())
		{
			return true;
		}
	}

	return false;
}

UMVDialoguePopup* UMVUISubsystem::ShowDialoguePopupText(FText DialogueText, float Duration)
{
	return ShowDialoguePopupTextWithTiming(DialogueText, Duration);
}

UMVDialoguePopup* UMVUISubsystem::ShowDialoguePopupTextWithTiming(FText DialogueText, float Duration, float MinimumSkipDelay)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->DialoguePopupClass)
	{
		return nullptr;
	}

	if (IsDialoguePopupBlockingInteraction())
	{
		return ActiveDialoguePopup;
	}

	if (IsPopupActive(ActivePopup))
	{
		QueueDialoguePopupText(DialogueText, Duration, MinimumSkipDelay);
		CloseActivePopup();
		return nullptr;
	}

	return OpenDialoguePopupText(DialogueText, Duration, MinimumSkipDelay);
}

UMVLoadingWindow* UMVUISubsystem::ShowLoadingWindowForTest(const bool bUseNativeWindow)
{
#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return nullptr;
	}

	ClearAllUI(true);
	HideHUD();

	UCommonActivatableWidget* LoadingWidget = bUseNativeWindow
		? PushWindowByClass(UMVLoadingWindow::StaticClass())
		: ShowLoadingWindow();
	ActiveLoadingWindowForTest = Cast<UMVLoadingWindow>(LoadingWidget);
	if (!ActiveLoadingWindowForTest)
	{
		return nullptr;
	}

	ActiveLoadingWindowForTest->LoadLoadingGuideCards();
	ActiveLoadingWindowForTest->SetLoadingProgress(
		0.35f,
		NSLOCTEXT("MaverickLoading", "LoadingTestStepText", "Loading screen test"));
	ActiveLoadingWindowForTest->SetKeyboardFocus();
	return ActiveLoadingWindowForTest;
#else
	return nullptr;
#endif
}

void UMVUISubsystem::HideLoadingWindowForTest()
{
#if !UE_BUILD_SHIPPING
	ActiveLoadingWindowForTest = nullptr;
	ResetToDefaultUI();

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

bool UMVUISubsystem::AdvanceLoadingGuideCardForTest()
{
#if !UE_BUILD_SHIPPING
	return ActiveLoadingWindowForTest
		? ActiveLoadingWindowForTest->AdvanceLoadingGuideCard()
		: false;
#else
	return false;
#endif
}

UMVDialoguePopup* UMVUISubsystem::OpenDialoguePopupText(FText DialogueText, float Duration, float MinimumSkipDelay)
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || !Settings->DialoguePopupClass)
	{
		return nullptr;
	}

	const float DisplayDuration = Duration >= 0.0f ? Duration : Settings->DialoguePopupDuration;
	const float ResolvedMinimumSkipDelay = MinimumSkipDelay >= 0.0f
		? MinimumSkipDelay
		: Settings->DialoguePopupMinimumSkipDelay;
	if (IsDialoguePopupPresent(ActiveDialoguePopup))
	{
		return ActiveDialoguePopup;
	}

	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer)
	{
		return nullptr;
	}

	ActiveDialoguePopup = Cast<UMVDialoguePopup>(Layer->PushPopup(Settings->DialoguePopupClass));
	if (ActiveDialoguePopup)
	{
		TrackActiveDialoguePopup(ActiveDialoguePopup);
		ApplyDialogueCameraZoom(ActiveDialoguePopup->GetDialogueFadeSeconds());
		ActiveDialoguePopup->SetMinimumSkipDelay(ResolvedMinimumSkipDelay);
		ActiveDialoguePopup->SetDialogueAutoDismissSeconds(DisplayDuration);
		ActiveDialoguePopup->SetDialogueText(DialogueText);
	}

	bHasPendingDialogueRequest = false;
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;
	PendingDialogueMinimumSkipDelay = -1.0f;
	return ActiveDialoguePopup;
}

void UMVUISubsystem::HideDialoguePopup()
{
	bHasPendingDialogueRequest = false;
	PendingDialogueText = FText::GetEmpty();
	PendingDialogueDuration = -1.0f;
	PendingDialogueMinimumSkipDelay = -1.0f;

	if (IsDialoguePopupPresent(ActiveDialoguePopup))
	{
		ActiveDialoguePopup->CloseDialogue();
		return;
	}

	ActiveDialoguePopup = nullptr;
}

void UMVUISubsystem::SkipDialoguePopup()
{
	if (CanSkipDialoguePopup())
	{
		HideDialoguePopup();
	}
}

bool UMVUISubsystem::CanSkipDialoguePopup() const
{
	return IsDialoguePopupPresent(ActiveDialoguePopup)
		&& ActiveDialoguePopup->CanSkipDialogue();
}

bool UMVUISubsystem::CanUseInteractionPrompt() const
{
	return !IsDialoguePopupBlockingInteraction()
		&& !IsInteractionMenuActive()
		&& !IsInteractionChoiceActive()
		&& !IsInteractionSessionActive()
		&& IsPopupActive(ActiveInteractionPrompt)
		&& ActivePopup == ActiveInteractionPrompt
		&& !ActiveInteractionPrompt->IsClosing()
		&& !ActiveInteractionPrompt->IsFading();
}

bool UMVUISubsystem::IsDialoguePopupActive() const
{
	return IsDialoguePopupActive(ActiveDialoguePopup);
}

bool UMVUISubsystem::IsDialoguePopupBlockingInteraction() const
{
	return bHasPendingDialogueRequest
		|| bDialoguePromptRestoreDelayActive
		|| IsDialoguePopupPresent(ActiveDialoguePopup);
}

UMVDialoguePopup* UMVUISubsystem::ShowDialoguePopupByRow(FDataTableRowHandle DialogueRow)
{
	if (!DialogueRow.DataTable || DialogueRow.RowName.IsNone())
	{
		UE_LOG(
			LogMVUISubsystem,
			Warning,
			TEXT("Dialogue window by row failed. DialogueRow is incomplete. DataTable=%s, RowName=%s."),
			*GetNameSafe(DialogueRow.DataTable),
			*DialogueRow.RowName.ToString());
		return nullptr;
	}

	const UScriptStruct* RowStruct = DialogueRow.DataTable->GetRowStruct();
	if (!RowStruct || !RowStruct->IsChildOf(FMVDialogueRow::StaticStruct()))
	{
		UE_LOG(
			LogMVUISubsystem,
			Warning,
			TEXT("Dialogue window by row failed. DataTable '%s' uses invalid row struct '%s'. Expected MVDialogueRow."),
			*GetNameSafe(DialogueRow.DataTable),
			RowStruct ? *RowStruct->GetName() : TEXT("None"));
		return nullptr;
	}

	const FMVDialogueRow* FoundDialogueRow = DialogueRow.DataTable->FindRow<FMVDialogueRow>(
		DialogueRow.RowName,
		TEXT("MVUISubsystem"),
		false);
	if (!FoundDialogueRow)
	{
		UE_LOG(
			LogMVUISubsystem,
			Warning,
			TEXT("Dialogue window by row failed. Row '%s' was not found in table '%s'."),
			*DialogueRow.RowName.ToString(),
			*GetNameSafe(DialogueRow.DataTable));
		return nullptr;
	}

	return ShowDialoguePopupTextWithTiming(
		FoundDialogueRow->DialogueText,
		FoundDialogueRow->DisplayDuration,
		FoundDialogueRow->MinimumSkipDelay);
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

void UMVUISubsystem::ClearAllUI(bool bUseFadeOut)
{
	ClearAllUIInternal(bUseFadeOut);
}

void UMVUISubsystem::ClearAllUIInternal(bool bUseFadeOut)
{
	for (UMVUILayerBase* Layer : LayerStack)
	{
		if (!Layer)
		{
			continue;
		}

		if (bUseFadeOut)
		{
			Layer->SetVisibility(ESlateVisibility::HitTestInvisible);
			Layer->RemoveFromParentWithFade();
		}
		else
		{
			Layer->ClearLayer();
			Layer->RemoveFromParent();
		}
	}

	LayerStack.Reset();
	ResetUITrackingState();
}

void UMVUISubsystem::ResetUITrackingState()
{
	CachedHUD = nullptr;
	ActiveInteractionPrompt = nullptr;
	ActiveInteractionMenuWindow = nullptr;
	ActiveInteractionChoicePopup = nullptr;
	ActiveDialoguePopup = nullptr;
	ActivePopup = nullptr;
	ActiveLoadingWindowForTest = nullptr;
	ActiveInteractionMenuSource.Reset();
	ActiveInteractionChoiceSource.Reset();
	InteractionSessionSources.Reset();
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

void UMVUISubsystem::ResetToDefaultUI()
{
	UWorld* World = GetWorld();
	ClearAllUIInternal(true);

	if (!World || !World->IsGameWorld())
	{
		return;
	}

	GetOrCreateRootLayer(World);
	ShowDefaultHUD();
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
		}),
		0.1f,
		false);
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

	TryOpenPendingDialoguePopup();
}

void UMVUISubsystem::HandleInteractionMenuClosed(UMVInteractionMenuWindow* ClosedMenuWindow)
{
	if (!ClosedMenuWindow)
	{
		return;
	}

	ClosedMenuWindow->OnInteractionMenuClosed.RemoveDynamic(this, &UMVUISubsystem::HandleInteractionMenuClosed);

	if (ClosedMenuWindow == ActiveInteractionMenuWindow)
	{
		UObject* SessionSource = ActiveInteractionMenuSource.Get();
		EndInteractionSession(SessionSource ? SessionSource : ClosedMenuWindow);
		ActiveInteractionMenuWindow = nullptr;
		ActiveInteractionMenuSource.Reset();
	}
}

void UMVUISubsystem::HandleInteractionChoiceClosed(UMVPopupBase* ClosedChoicePopup)
{
	if (!ClosedChoicePopup)
	{
		return;
	}

	ClosedChoicePopup->OnPopupClosed.RemoveDynamic(
		this,
		&UMVUISubsystem::HandleInteractionChoiceClosed);

	if (ClosedChoicePopup == ActiveInteractionChoicePopup)
	{
		UObject* SessionSource = ActiveInteractionChoiceSource.Get();
		EndInteractionSession(SessionSource ? SessionSource : ClosedChoicePopup);
		ActiveInteractionChoicePopup = nullptr;
		ActiveInteractionChoiceSource.Reset();
	}
}

void UMVUISubsystem::HandleDialoguePopupClosed(UMVDialoguePopup* ClosedDialoguePopup)
{
	if (!ClosedDialoguePopup)
	{
		return;
	}

	ClosedDialoguePopup->OnDialoguePopupClosed.RemoveDynamic(this, &UMVUISubsystem::HandleDialoguePopupClosed);
	ClosedDialoguePopup->OnDialoguePopupClosing.RemoveDynamic(this, &UMVUISubsystem::HandleDialoguePopupClosing);

	if (ClosedDialoguePopup == ActiveDialoguePopup)
	{
		ActiveDialoguePopup = nullptr;
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

}

void UMVUISubsystem::HandleDialoguePopupClosing(UMVDialoguePopup* ClosingDialoguePopup)
{
	const float DurationSeconds = IsValid(ClosingDialoguePopup)
		? ClosingDialoguePopup->GetDialogueFadeSeconds()
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

	if (IsValid(ActiveDialoguePopup))
	{
		return ActiveDialoguePopup->GetDialogueFadeSeconds();
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

bool UMVUISubsystem::IsDialoguePopupActive(const UMVDialoguePopup* DialoguePopup) const
{
	return IsPopupActive(DialoguePopup) && !DialoguePopup->IsClosing();
}

bool UMVUISubsystem::IsDialoguePopupPresent(const UMVDialoguePopup* DialoguePopup) const
{
	return IsPopupActive(DialoguePopup);
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

void UMVUISubsystem::QueueDialoguePopupText(FText DialogueText, float Duration, float MinimumSkipDelay)
{
	PendingDialogueText = DialogueText;
	PendingDialogueDuration = Duration;
	PendingDialogueMinimumSkipDelay = MinimumSkipDelay;
	bHasPendingDialogueRequest = true;
}

void UMVUISubsystem::TryOpenPendingDialoguePopup()
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

	OpenDialoguePopupText(DialogueText, DialogueDuration, DialogueMinimumSkipDelay);
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

void UMVUISubsystem::TrackActiveDialoguePopup(UMVDialoguePopup* DialoguePopup)
{
	if (IsValid(ActiveDialoguePopup))
	{
		ActiveDialoguePopup->OnDialoguePopupClosing.RemoveDynamic(this, &UMVUISubsystem::HandleDialoguePopupClosing);
		ActiveDialoguePopup->OnDialoguePopupClosed.RemoveDynamic(this, &UMVUISubsystem::HandleDialoguePopupClosed);
	}

	ActiveDialoguePopup = DialoguePopup;
	if (IsValid(ActiveDialoguePopup))
	{
		ActiveDialoguePopup->OnDialoguePopupClosing.AddUniqueDynamic(this, &UMVUISubsystem::HandleDialoguePopupClosing);
		ActiveDialoguePopup->OnDialoguePopupClosed.AddUniqueDynamic(this, &UMVUISubsystem::HandleDialoguePopupClosed);
	}
}
