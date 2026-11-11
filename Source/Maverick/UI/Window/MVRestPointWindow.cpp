#include "UI/Window/MVRestPointWindow.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"
#include "InputCoreTypes.h"
#include "CommonInputSubsystem.h"
#include "UI/System/MVMenuInputHint.h"
#include "Interaction/RestPoint/MVRestPoint.h"
#include "Interaction/RestPoint/MVRestPointSubsystem.h"
#include "System/MVWorldStateSubsystem.h"

namespace
{
const FLinearColor MVRestMenuIvory(0.88f, 0.81f, 0.67f);
const FLinearColor MVRestMenuMuted(0.56f, 0.51f, 0.43f);
const FLinearColor MVRestMenuButtonColor(0.42f, 0.28f, 0.13f);
const FLinearColor MVRestMenuSelectedButtonColor(0.77f, 0.57f, 0.26f);

UTextBlock* MVRestMenuMakeLabel(UWidgetTree& Tree, const FText& Text, int32 Size, const FLinearColor& Color)
{
	UTextBlock* Label = Tree.ConstructWidget<UTextBlock>();
	Label->SetText(Text);
	FSlateFontInfo Font = Label->GetFont();
	Font.Size = Size;
	Label->SetFont(Font);
	Label->SetColorAndOpacity(FSlateColor(Color));
	Label->SetAutoWrapText(true);
	Label->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Label;
}
}

UMVRestPointWindow::UMVRestPointWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UMVRestPointWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildNativeMenu();
	if (RestButton)
	{
		RestButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleRestClicked);
	}
	if (LevelUpButton)
	{
		LevelUpButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleLevelUpClicked);
	}
	if (PassiveSkillsButton)
	{
		PassiveSkillsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandlePassiveSkillsClicked);
	}
	if (RetrySaveButton)
	{
		RetrySaveButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleRetrySaveClicked);
	}
	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleLeaveClicked);
	}
}

void UMVRestPointWindow::NativeOnActivated()
{
	Super::NativeOnActivated();

	UWorld* World = GetWorld();
	UMVRestPointSubsystem* Session = World ? World->GetSubsystem<UMVRestPointSubsystem>() : nullptr;
	const AMVRestPoint* RestPoint = Session ? Session->GetActiveRestPoint() : nullptr;
	bool bReturningFromDetail = false;
	if (!RestPoint || !Session->NotifyMenuActivated(*this, bReturningFromDetail))
	{
		DeactivateWidget();
		return;
	}

	if (RestPointName)
	{
		RestPointName->SetText(RestPoint->GetDisplayName());
	}
	if (!bReturningFromDetail)
	{
		SetMenuSelection(LeaveButton);
	}
	bShowingSaveFailure = false;
	ShowRestStatus(Session->GetRestStatus());
	RefreshInputHints();
	if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		Input->OnInputMethodChangedNative.RemoveAll(this);
		Input->OnInputMethodChangedNative.AddWeakLambda(this, [this](ECommonInputType) { RefreshInputHints(); });
	}

	SetVisibility(ESlateVisibility::Visible);
	const FDataTableRowHandle ConfirmAction = ICommonInputModule::GetSettings().GetDefaultClickAction();
	if (!ConfirmAction.IsNull())
	{
		FBindUIActionArgs Args(ConfirmAction, false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleConfirmInput));
		Args.InputMode = ECommonInputMode::Menu;
		Args.bConsumeInput = true;
		ConfirmInputBinding = RegisterUIActionBinding(Args);
	}

}

void UMVRestPointWindow::NativeOnDeactivated()
{
	if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		Input->OnInputMethodChangedNative.RemoveAll(this);
	}
	UnregisterConfirmInput();
	if (UWorld* World = GetWorld())
	{
		if (UMVRestPointSubsystem* Session = World->GetSubsystem<UMVRestPointSubsystem>())
		{
			Session->NotifyMenuDeactivated(*this);
		}
	}
	Super::NativeOnDeactivated();
}

void UMVRestPointWindow::NativeDestruct()
{
	if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		Input->OnInputMethodChangedNative.RemoveAll(this);
	}
	UnregisterConfirmInput();
	Super::NativeDestruct();
}

void UMVRestPointWindow::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	if (!IsActivated())
	{
		return;
	}
	RefreshSaveStatus();

	APlayerController* Controller = GetOwningPlayer();
	if (!Controller || HasUserFocus(Controller) || HasUserFocusedDescendants(Controller)
		|| !FSlateApplication::IsInitialized())
	{
		return;
	}

	// 게임 뷰포트나 미리보기 창에 돌아오면 목록의 초점을 복구한다.
	FSlateApplication& Slate = FSlateApplication::Get();
	const TSharedPtr<SWidget> MenuWidget = GetCachedWidget();
	const TSharedPtr<SWindow> ActiveWindow = Slate.GetActiveTopLevelWindow();
	if (MenuWidget && ActiveWindow && Slate.FindWidgetWindow(MenuWidget.ToSharedRef()) == ActiveWindow)
	{
		NativeGetDesiredFocusTarget()->SetUserFocus(Controller);
	}
}

UWidget* UMVRestPointWindow::NativeGetDesiredFocusTarget() const
{
	if (UButton* Button = SelectedButton.Get())
	{
		return Button;
	}
	return LeaveButton ? static_cast<UWidget*>(LeaveButton.Get()) : const_cast<UMVRestPointWindow*>(this);
}

FReply UMVRestPointWindow::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	const FKey Key = Event.GetKey();
	EUINavigation Direction = EUINavigation::Invalid;
	if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up)
	{
		Direction = EUINavigation::Up;
	}
	else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down)
	{
		Direction = EUINavigation::Down;
	}
	if (UButton* Target = MoveMenuSelection(Direction))
	{
		return FReply::Handled().SetUserFocus(Target->TakeWidget(), EFocusCause::Navigation);
	}
	return Super::NativeOnPreviewKeyDown(Geometry, Event);
}

FNavigationReply UMVRestPointWindow::NativeOnNavigation(
	const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply)
{
	if (UButton* Target = MoveMenuSelection(Event.GetNavigationType()))
	{
		return FNavigationReply::Explicit(Target->TakeWidget());
	}
	return Super::NativeOnNavigation(Geometry, Event, DefaultReply);
}

FReply UMVRestPointWindow::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
	FReply Reply = Super::NativeOnMouseButtonDown(Geometry, Event);
	if (!Reply.IsEventHandled() && IsActivated())
	{
		if (UWidget* FocusTarget = NativeGetDesiredFocusTarget())
		{
			return FReply::Handled().SetUserFocus(FocusTarget->TakeWidget(), EFocusCause::Mouse);
		}
	}
	return Reply;
}

void UMVRestPointWindow::HandleLeaveClicked()
{
	if (IsActivated() && !IsRestPending())
	{
		DeactivateWidgetWithFade();
	}
}

bool UMVRestPointWindow::NativeOnHandleBackAction()
{
	return IsRestPending() || Super::NativeOnHandleBackAction();
}

bool UMVRestPointWindow::IsRestPending() const
{
	const UWorld* World = GetWorld();
	const UMVRestPointSubsystem* Session = World ? World->GetSubsystem<UMVRestPointSubsystem>() : nullptr;
	return Session && Session->IsRestPending();
}

void UMVRestPointWindow::ShowRestStatus(const FText& Status)
{
	RefreshSaveStatus();
	if (MenuMessage && (!Status.IsEmpty() || !bShowingSaveFailure))
	{
		MenuMessage->SetText(Status);
	}
}

void UMVRestPointWindow::HandleRestClicked()
{
	UWorld* World = GetWorld();
	UMVRestPointSubsystem* Session = World ? World->GetSubsystem<UMVRestPointSubsystem>() : nullptr;
	if (!Session || Session->IsRestPending())
	{
		return;
	}
	SetMenuSelection(RestButton);
	if (RestButton)
	{
		RestButton->SetUserFocus(GetOwningPlayer());
	}
	if (Session->RequestRest(*this))
	{
		ShowRestStatus(Session->GetRestStatus());
	}
	else
	{
		ShowRestStatus(NSLOCTEXT("MVRestPoint", "CannotRest", "지금은 휴식할 수 없습니다"));
	}
}

void UMVRestPointWindow::HandleConfirmInput()
{
	UButton* Button = GetCurrentMenuButton();
	if (Button && Button == RestButton)
	{
		HandleRestClicked();
	}
	else if (Button && Button == LevelUpButton)
	{
		HandleLevelUpClicked();
	}
	else if (Button && Button == PassiveSkillsButton)
	{
		HandlePassiveSkillsClicked();
	}
	else if (Button && Button == RetrySaveButton)
	{
		HandleRetrySaveClicked();
	}
	else
	{
		HandleLeaveClicked();
	}
}

UButton* UMVRestPointWindow::GetCurrentMenuButton() const
{
	APlayerController* Controller = GetOwningPlayer();
	for (UButton* Button : { RestButton.Get(), LevelUpButton.Get(), PassiveSkillsButton.Get(), RetrySaveButton.Get(), LeaveButton.Get() })
	{
		if (Button && Button->HasUserFocus(Controller))
		{
			return Button;
		}
	}
	return SelectedButton.IsValid() ? SelectedButton.Get() : LeaveButton.Get();
}

UButton* UMVRestPointWindow::MoveMenuSelection(EUINavigation Direction)
{
	if (!IsActivated() || IsFadingOut() || (Direction != EUINavigation::Up && Direction != EUINavigation::Down))
	{
		return nullptr;
	}

	TArray<UButton*, TInlineAllocator<5>> Buttons;
	for (UButton* Button : { RestButton.Get(), LevelUpButton.Get(), PassiveSkillsButton.Get(), RetrySaveButton.Get(), LeaveButton.Get() })
	{
		if (Button && Button->GetIsEnabled() && Button->IsVisible())
		{
			Buttons.Add(Button);
		}
	}
	if (Buttons.IsEmpty())
	{
		return nullptr;
	}

	const int32 CurrentIndex = Buttons.IndexOfByKey(GetCurrentMenuButton());
	const int32 Step = Direction == EUINavigation::Down ? 1 : -1;
	const int32 NextIndex = FMath::Clamp(CurrentIndex + Step, 0, Buttons.Num() - 1);
	UButton* Target = Buttons[NextIndex];
	SetMenuSelection(Target);
	return Target;
}

void UMVRestPointWindow::SetMenuSelection(UButton* Button)
{
	if (bUsesNativeMenu)
	{
		if (UButton* Previous = SelectedButton.Get())
		{
			Previous->SetBackgroundColor(MVRestMenuButtonColor);
		}
		if (Button)
		{
			Button->SetBackgroundColor(MVRestMenuSelectedButtonColor);
		}
	}
	SelectedButton = Button;
}

void UMVRestPointWindow::HandleLevelUpClicked()
{
	OpenDetail(EMVRestPointDetail::LevelUp, LevelUpButton);
}

void UMVRestPointWindow::HandlePassiveSkillsClicked()
{
	OpenDetail(EMVRestPointDetail::PassiveSkills, PassiveSkillsButton);
}

void UMVRestPointWindow::RefreshSaveStatus()
{
	const UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
	const bool bFailed = State && State->HasFailedActiveSave();
	if (RetrySaveButton)
	{
		const ESlateVisibility RetryVisibility = bFailed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		if (RetrySaveButton->GetVisibility() != RetryVisibility)
		{
			RetrySaveButton->SetVisibility(RetryVisibility);
			if (!bFailed && SelectedButton.Get() == RetrySaveButton)
			{
				SetMenuSelection(LeaveButton);
			}
		}
	}
	if (MenuMessage && bFailed != bShowingSaveFailure)
	{
		MenuMessage->SetText(bFailed
			? NSLOCTEXT("MVRestPoint", "SaveFailed", "저장하지 못했습니다. 지금 종료하면 저장되지 않은 진행을 잃을 수 있습니다")
			: FText::GetEmpty());
	}
	bShowingSaveFailure = bFailed;
}

void UMVRestPointWindow::HandleRetrySaveClicked()
{
	UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
	if (IsActivated() && !IsFadingOut() && !IsRestPending() && State && State->HasFailedActiveSave() && State->SaveActiveSlot())
	{
		RefreshSaveStatus();
		if (MenuMessage)
		{
			MenuMessage->SetText(NSLOCTEXT("MVRestPoint", "SaveSucceeded", "저장했습니다"));
		}
	}
}

void UMVRestPointWindow::OpenDetail(EMVRestPointDetail Detail, UButton* Button)
{
	UWorld* World = GetWorld();
	UMVRestPointSubsystem* Session = World ? World->GetSubsystem<UMVRestPointSubsystem>() : nullptr;
	SetMenuSelection(Button);
	if (Button)
	{
		Button->SetUserFocus(GetOwningPlayer());
	}
	const bool bOpened = Session && Session->OpenDetail(*this, Detail);
	if (MenuMessage && !bShowingSaveFailure)
	{
		MenuMessage->SetText(bOpened ? FText::GetEmpty()
			: NSLOCTEXT("MVRestPoint", "DetailUnavailable", "지금은 이 메뉴를 열 수 없습니다"));
	}
}

void UMVRestPointWindow::UnregisterConfirmInput()
{
	if (ConfirmInputBinding.IsValid())
	{
		RemoveActionBinding(ConfirmInputBinding);
		ConfirmInputBinding.Unregister();
		ConfirmInputBinding = {};
	}
}

void UMVRestPointWindow::RefreshInputHints()
{
	if (!InputGuide)
	{
		return;
	}
	const UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
	const FText NavigationHint = Input && Input->GetCurrentInputType() == ECommonInputType::Gamepad
		? NSLOCTEXT("MVRestPoint", "GamepadNavigation", "십자키 위·아래 이동")
		: NSLOCTEXT("MVRestPoint", "KeyboardNavigation", "↑ ↓ 이동");
	const UCommonInputSettings& Settings = ICommonInputModule::GetSettings();
	InputGuide->SetText(FText::Format(NSLOCTEXT("MVRestPoint", "InputGuide", "{0} · {1} · {2}"), NavigationHint,
		MVMenuInputHint::MakeActionLabel(GetOwningLocalPlayer(), Settings.GetDefaultClickAction(), NSLOCTEXT("MVRestPoint", "ConfirmHint", "선택")),
		MVMenuInputHint::MakeActionLabel(GetOwningLocalPlayer(), Settings.GetDefaultBackAction(), NSLOCTEXT("MVRestPoint", "BackHint", "돌아가기"))));
}

void UMVRestPointWindow::BuildNativeMenu()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}
	bUsesNativeMenu = true;

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Canvas;
	UBorder* Shade = WidgetTree->ConstructWidget<UBorder>();
	Shade->SetBrushColor(FLinearColor(0.015f, 0.012f, 0.010f, 0.40f));
	Shade->SetVisibility(ESlateVisibility::HitTestInvisible);
	UCanvasPanelSlot* ShadeSlot = Canvas->AddChildToCanvas(Shade);
	ShadeSlot->SetAnchors(FAnchors(0, 0, 1, 1));
	ShadeSlot->SetOffsets(FMargin(0));

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetBrushColor(FLinearColor(0.024f, 0.019f, 0.014f, 0.96f));
	Panel->SetPadding(FMargin(24.0f));
	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.05f, 0.12f, 0.42f, 0.84f));
	PanelSlot->SetOffsets(FMargin(0));

	UVerticalBox* Entries = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Entries);
	Entries->AddChildToVerticalBox(MVRestMenuMakeLabel(*WidgetTree,
		NSLOCTEXT("MVRestPoint", "Title", "휴식 거점"), 34, MVRestMenuIvory));

	RestPointName = MVRestMenuMakeLabel(*WidgetTree, FText::GetEmpty(), 23, MVRestMenuIvory);
	Entries->AddChildToVerticalBox(RestPointName)->SetPadding(FMargin(0, 8, 0, 24));

	const auto AddButton = [this, Entries](const FText& Label)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>();
		Button->SetBackgroundColor(MVRestMenuButtonColor);
		UTextBlock* ButtonLabel = MVRestMenuMakeLabel(*WidgetTree, Label, 23, MVRestMenuIvory);
		ButtonLabel->SetAutoWrapText(false);
		Button->SetContent(ButtonLabel);
		Entries->AddChildToVerticalBox(Button)->SetPadding(FMargin(0, 8, 0, 8));
		return Button;
	};
	RestButton = AddButton(NSLOCTEXT("MVRestPoint", "Rest", "휴식하기"));
	LevelUpButton = AddButton(NSLOCTEXT("MVRestPoint", "LevelUp", "레벨업"));
	PassiveSkillsButton = AddButton(NSLOCTEXT("MVRestPoint", "PassiveSkills", "패시브 스킬"));
	RetrySaveButton = AddButton(NSLOCTEXT("MVRestPoint", "RetrySave", "저장 다시 시도"));
	RetrySaveButton->SetVisibility(ESlateVisibility::Collapsed);
	LeaveButton = AddButton(NSLOCTEXT("MVRestPoint", "Leave", "떠나기"));
	MenuMessage = MVRestMenuMakeLabel(*WidgetTree, FText::GetEmpty(), 18, MVRestMenuIvory);
	Entries->AddChildToVerticalBox(MenuMessage)->SetPadding(FMargin(0, 8, 0, 0));
	InputGuide = MVRestMenuMakeLabel(*WidgetTree, FText::GetEmpty(), 16, MVRestMenuMuted);
	Entries->AddChildToVerticalBox(InputGuide)->SetPadding(FMargin(0, 16, 0, 0));
}
