#include "UI/Window/MVRestWeaponSlotWindow.h"

#include "Blueprint/WidgetTree.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/MVWeaponComponent.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"
#include "Interaction/RestPoint/MVRestPointSubsystem.h"
#include "UI/Window/MVInventoryPage.h"

#define LOCTEXT_NAMESPACE "MVRestWeaponSlots"

namespace
{
UTextBlock* MVRestWeaponSlotLabel(UWidgetTree& Tree, const FText& Caption, const int32 FontSize,
	const FName Name = NAME_None)
{
	auto* Label = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Label->SetText(Caption);
	Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.91f, 0.85f, 0.74f)));
	FSlateFontInfo Font = Label->GetFont();
	Font.Size = FontSize;
	Label->SetFont(Font);
	Label->SetAutoWrapText(true);
	Label->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Label;
}

FText MVRestWeaponSlotName(const FGameplayTag Tag, const TMap<FGameplayTag, FDataTableRowHandle>& Owned)
{
	if (!Tag.IsValid())
	{
		return LOCTEXT("EmptySlot", "비어 있음");
	}
	const auto* Handle = Owned.Find(Tag);
	const auto* Row = Handle ? Handle->GetRow<FMVWeaponTableRow>(TEXT("Rest point weapon list")) : nullptr;
	return Row && !Row->DisplayName.IsEmpty() ? Row->DisplayName : FText::FromName(Tag.GetTagName());
}
}

UMVRestWeaponSlotWindow::UMVRestWeaponSlotWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UMVRestWeaponSlotWindow::SetSession(UMVRestPointSubsystem& RestPointSession, AMVPlayerCharacter& Player)
{
	Session = &RestPointSession;
	Weapons = Player.WeaponComponent;
	SelectedSlotIndex = Weapons.IsValid() ? FMath::Max(0, Weapons->GetActiveQuickSlotIndex()) : INDEX_NONE;
}

void UMVRestWeaponSlotWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Canvas;
	auto* Shade = WidgetTree->ConstructWidget<UBorder>();
	Shade->SetBrushColor(FLinearColor(0.015f, 0.012f, 0.010f, 0.55f));
	Shade->SetVisibility(ESlateVisibility::HitTestInvisible);
	auto* ShadePlacement = Canvas->AddChildToCanvas(Shade);
	ShadePlacement->SetAnchors(FAnchors(0, 0, 1, 1));
	ShadePlacement->SetOffsets(FMargin(0));

	auto* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetBrushColor(FLinearColor(0.030f, 0.025f, 0.021f, 0.97f));
	Panel->SetPadding(FMargin(28.0f));
	auto* PanelPlacement = Canvas->AddChildToCanvas(Panel);
	PanelPlacement->SetAnchors(FAnchors(0.12f, 0.13f, 0.88f, 0.87f));
	PanelPlacement->SetOffsets(FMargin(0));

	auto* Body = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Body);
	Body->AddChildToVerticalBox(MVRestWeaponSlotLabel(*WidgetTree, LOCTEXT("Title", "무기 퀵슬롯"), 34))
		->SetPadding(FMargin(0, 0, 0, 20));

	auto* Columns = WidgetTree->ConstructWidget<UHorizontalBox>();
	Body->AddChildToVerticalBox(Columns)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	const auto AddList = [this, Columns](const FName Name, const FText& Caption, const float Weight)
	{
		auto* Column = WidgetTree->ConstructWidget<UVerticalBox>();
		auto* Placement = Columns->AddChildToHorizontalBox(Column);
		FSlateChildSize Size(ESlateSizeRule::Fill);
		Size.Value = Weight;
		Placement->SetSize(Size);
		Placement->SetPadding(FMargin(0, 0, 16, 0));
		Column->AddChildToVerticalBox(MVRestWeaponSlotLabel(*WidgetTree, Caption, 20))
			->SetPadding(FMargin(0, 0, 0, 8));
		auto* List = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), Name);
		List->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::InstantScroll);
		Column->AddChildToVerticalBox(List)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		return List;
	};
	SlotList = AddList(TEXT("WeaponSlots"), LOCTEXT("Slots", "슬롯"), 2.0f);
	WeaponList = AddList(TEXT("OwnedWeaponChoices"), LOCTEXT("Weapons", "보유 무기"), 3.0f);

	StatusText = MVRestWeaponSlotLabel(*WidgetTree, FText::GetEmpty(), 17, TEXT("WeaponSlotStatus"));
	Body->AddChildToVerticalBox(StatusText)->SetPadding(FMargin(0, 10, 0, 10));
	BackButton = MakeButton(LOCTEXT("Back", "돌아가기"),
		FSimpleDelegate::CreateUObject(this, &ThisClass::DeactivateWidgetWithFade), TEXT("WeaponSlotBack"));
	Body->AddChildToVerticalBox(BackButton)->SetHorizontalAlignment(HAlign_Left);
}

void UMVRestWeaponSlotWindow::NativeOnActivated()
{
	Super::NativeOnActivated();
	if (!Session.IsValid() || !Weapons.IsValid())
	{
		DeactivateWidget();
		return;
	}

	Weapons->OnQuickSlotsChanged.AddUniqueDynamic(this, &ThisClass::HandleWeaponsChanged);
	Weapons->OnOwnedWeaponsChanged.AddUniqueDynamic(this, &ThisClass::HandleWeaponsChanged);
	StatusText->SetText(FText::GetEmpty());
	RefreshChoices();

	const FDataTableRowHandle Confirm = ICommonInputModule::GetSettings().GetDefaultClickAction();
	if (!Confirm.IsNull())
	{
		FBindUIActionArgs Args(Confirm, false, FSimpleDelegate::CreateUObject(this, &ThisClass::ConfirmFocusedChoice));
		Args.InputMode = ECommonInputMode::Menu;
		Args.bConsumeInput = true;
		ConfirmBinding = RegisterUIActionBinding(Args);
	}
}

void UMVRestWeaponSlotWindow::NativeOnDeactivated()
{
	UnbindWindow();
	Super::NativeOnDeactivated();
}

void UMVRestWeaponSlotWindow::NativeDestruct()
{
	UnbindWindow();
	Super::NativeDestruct();
}

void UMVRestWeaponSlotWindow::UnbindWindow()
{
	if (UMVWeaponComponent* Component = Weapons.Get())
	{
		Component->OnQuickSlotsChanged.RemoveDynamic(this, &ThisClass::HandleWeaponsChanged);
		Component->OnOwnedWeaponsChanged.RemoveDynamic(this, &ThisClass::HandleWeaponsChanged);
	}
	if (ConfirmBinding.IsValid())
	{
		RemoveActionBinding(ConfirmBinding);
		ConfirmBinding.Unregister();
		ConfirmBinding = {};
	}
}

UMVInventoryButton* UMVRestWeaponSlotWindow::MakeButton(const FText& Caption, FSimpleDelegate OnClick, const FName Name)
{
	auto* Button = WidgetTree->ConstructWidget<UMVInventoryButton>(UMVInventoryButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor(0.43f, 0.34f, 0.25f));
	Button->SetContent(MVRestWeaponSlotLabel(*WidgetTree, Caption, 21));
	Button->SetClickAction(MoveTemp(OnClick));
	return Button;
}

void UMVRestWeaponSlotWindow::RefreshChoices()
{
	++ChoiceRevision;
	SlotList->ClearChildren();
	WeaponList->ClearChildren();
	SlotButtons.Reset();
	WeaponButtons.Reset();

	const UMVWeaponComponent* Component = Weapons.Get();
	const TArray<FGameplayTag> Slots = Component ? Component->GetQuickSlotWeapons() : TArray<FGameplayTag>();
	if (Slots.IsEmpty())
	{
		SlotList->AddChild(MVRestWeaponSlotLabel(*WidgetTree, LOCTEXT("NoSlots", "사용 가능한 슬롯이 없습니다"), 19));
		SelectedSlotIndex = INDEX_NONE;
		return;
	}

	SelectedSlotIndex = FMath::Clamp(SelectedSlotIndex, 0, Slots.Num() - 1);
	const TMap<FGameplayTag, FDataTableRowHandle> Owned = Component->GetOwnedWeapons();
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		const FText Name = MVRestWeaponSlotName(Slots[Index], Owned);
		const FText Caption = FText::Format(Index == Component->GetActiveQuickSlotIndex()
			? LOCTEXT("ActiveSlot", "슬롯 {0} · {1} (현재 슬롯)") : LOCTEXT("Slot", "슬롯 {0} · {1}"),
			FText::AsNumber(Index + 1), Name);
		auto* Button = MakeButton(Caption, FSimpleDelegate::CreateUObject(this, &ThisClass::ChooseSlot, Index));
		if (Index == SelectedSlotIndex)
		{
			Button->SetBackgroundColor(FLinearColor(0.74f, 0.57f, 0.33f));
		}
		CastChecked<UScrollBoxSlot>(SlotList->AddChild(Button))->SetPadding(FMargin(0, 0, 6, 6));
		SlotButtons.Add(Button);
	}

	const FGameplayTag ExpectedWeapon = Slots[SelectedSlotIndex];
	const auto AddChoice = [this, ExpectedWeapon](const FText& Caption, const FGameplayTag Tag, const bool bAvailable)
	{
		auto* Button = MakeButton(Caption,
			FSimpleDelegate::CreateUObject(this, &ThisClass::CommitChoice, ChoiceRevision, SelectedSlotIndex, ExpectedWeapon, Tag));
		Button->SetIsEnabled(bAvailable);
		CastChecked<UScrollBoxSlot>(WeaponList->AddChild(Button))->SetPadding(FMargin(0, 0, 6, 6));
		WeaponButtons.Add(Button);
	};
	AddChoice(LOCTEXT("ClearSlot", "슬롯 비우기"), FGameplayTag(), ExpectedWeapon.IsValid());

	TArray<FGameplayTag> Tags;
	Owned.GetKeys(Tags);
	Tags.Sort([](const FGameplayTag& A, const FGameplayTag& B) { return A.GetTagName().LexicalLess(B.GetTagName()); });
	for (const FGameplayTag Tag : Tags)
	{
		const int32 AssignedIndex = Slots.Find(Tag);
		const FText Name = MVRestWeaponSlotName(Tag, Owned);
		AddChoice(AssignedIndex == INDEX_NONE ? Name
			: FText::Format(LOCTEXT("AlreadyAssigned", "{0} (슬롯 {1})"), Name, FText::AsNumber(AssignedIndex + 1)),
			Tag, AssignedIndex == INDEX_NONE);
	}
	if (Tags.IsEmpty())
	{
		WeaponList->AddChild(MVRestWeaponSlotLabel(*WidgetTree, LOCTEXT("NoWeapons", "보유 무기가 없습니다"), 19));
	}
}

void UMVRestWeaponSlotWindow::ChooseSlot(const int32 SlotIndex)
{
	if (IsActivated() && !IsFadingOut() && SlotButtons.IsValidIndex(SlotIndex) && SelectedSlotIndex != SlotIndex)
	{
		SelectedSlotIndex = SlotIndex;
		HandleWeaponsChanged();
	}
}

void UMVRestWeaponSlotWindow::HandleWeaponsChanged()
{
	if (IsActivated() && !IsFadingOut())
	{
		RefreshChoices();
		NativeGetDesiredFocusTarget()->SetUserFocus(GetOwningPlayer());
	}
}

UWidget* UMVRestWeaponSlotWindow::NativeGetDesiredFocusTarget() const
{
	if (SlotButtons.IsValidIndex(SelectedSlotIndex))
	{
		return SlotButtons[SelectedSlotIndex];
	}
	return BackButton ? static_cast<UWidget*>(BackButton.Get()) : const_cast<UMVRestWeaponSlotWindow*>(this);
}

void UMVRestWeaponSlotWindow::ConfirmFocusedChoice()
{
	APlayerController* Controller = GetOwningPlayer();
	if (!Controller || !IsActivated() || IsFadingOut())
	{
		return;
	}
	for (const auto* Group : { &SlotButtons, &WeaponButtons })
	{
		for (UMVInventoryButton* Button : *Group)
		{
			if (Button->GetIsEnabled() && Button->HasUserFocus(Controller))
			{
				Button->OnClicked.Broadcast();
				return;
			}
		}
	}
	if (BackButton->HasUserFocus(Controller))
	{
		DeactivateWidgetWithFade();
	}
}

void UMVRestWeaponSlotWindow::CommitChoice(
	const uint32 Revision, const int32 SlotIndex, const FGameplayTag ExpectedWeapon, const FGameplayTag WeaponTag)
{
	UMVRestPointSubsystem* RestPointSession = Session.Get();
	if (!RestPointSession || Revision != ChoiceRevision || SlotIndex != SelectedSlotIndex || !IsActivated() || IsFadingOut())
	{
		return;
	}

	switch (RestPointSession->CommitWeaponSlot(*this, SlotIndex, ExpectedWeapon, WeaponTag))
	{
	case EMVRestWeaponSlotResult::Saved:
		StatusText->SetText(LOCTEXT("Saved", "무기 퀵슬롯을 저장했습니다"));
		break;
	case EMVRestWeaponSlotResult::SaveFailed:
		StatusText->SetText(LOCTEXT("SaveFailed", "편성은 적용했습니다. 거점 메뉴에서 저장을 다시 시도할 수 있습니다"));
		break;
	case EMVRestWeaponSlotResult::Rejected:
		StatusText->SetText(LOCTEXT("Rejected", "지금은 슬롯을 변경할 수 없습니다"));
		break;
	}
}

#undef LOCTEXT_NAMESPACE
