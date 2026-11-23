#include "UI/Window/MVInventoryPage.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/MVInventoryComponent.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Tables/MVItemTableTypes.h"
#include "Tables/MVTableManager.h"

#define LOCTEXT_NAMESPACE "MVInventoryPage"

namespace
{
const TOptional<EMVInventoryGroupCategory> MVInventoryPageCategories[] = {
	{}, EMVInventoryGroupCategory::Weapons, EMVInventoryGroupCategory::Consumables,
	EMVInventoryGroupCategory::Materials, EMVInventoryGroupCategory::Other
};
constexpr int32 MVInventoryPageColumns = 6;
constexpr float MVInventoryDetailsGap = 28.0f;
constexpr float MVInventoryHighlightHeight = 28.0f;

struct FMVInventoryPageEntry
{
	FGameplayTag Tag;
	int32 Quantity;
	const FMVInventoryItemDefinition* Definition;
	const FMVItemTableRow* Row;
};

FText MVInventoryPageCategoryName(const TOptional<EMVInventoryGroupCategory> Category)
{
	if (!Category.IsSet())
	{
		return LOCTEXT("All", "All Items");
	}
	switch (Category.GetValue())
	{
	case EMVInventoryGroupCategory::Weapons: return LOCTEXT("Weapons", "Weapons");
	case EMVInventoryGroupCategory::Consumables: return LOCTEXT("Consumables", "Consumables");
	case EMVInventoryGroupCategory::Materials: return LOCTEXT("Materials", "Materials");
	default: return LOCTEXT("Other", "Other");
	}
}

FText MVInventoryPageDisplayName(const FMVItemTableRow& Row)
{
	return Row.DisplayName.IsEmpty() ? FText::FromName(Row.ItemTag.GetTagName()) : Row.DisplayName;
}

UTexture2D* MVInventoryPageResolveIcon(const FMVItemTableRow& Row, UTexture2D* Fallback)
{
	UTexture2D* Icon = Row.Icon.LoadSynchronous();
	return Icon ? Icon : Fallback;
}

void MVInventoryPageSetItemFont(UTextBlock& TextBlock)
{
	FSlateFontInfo Font = TextBlock.GetFont();
	const FCompositeFont* Original = Font.GetCompositeFont();
	if (!Original || !Original->FallbackTypeface.Typeface.Fonts.IsEmpty())
	{
		return;
	}

	// 이식한 서체와 크기를 유지하고, 없는 글자는 엔진 기본 글꼴에서 찾는다.
	const TSharedRef<FStandaloneCompositeFont> Composite = MakeShared<FStandaloneCompositeFont>();
	static_cast<FCompositeFont&>(Composite.Get()) = *Original;
	Composite->FallbackTypeface = FCoreStyle::GetDefaultFont()->FallbackTypeface;
	Font.FontObject = nullptr;
	Font.CompositeFont = Composite;
	TextBlock.SetFont(Font);
}

FButtonStyle MVInventoryPageButtonStyle(FSlateBrush Background)
{
	Background.TintColor = FSlateColor(FLinearColor(0.56f, 0.49f, 0.40f, 1.0f));
	FSlateBrush Highlight = Background;
	Highlight.TintColor = FSlateColor(FLinearColor(0.88f, 0.77f, 0.59f, 1.0f));
	return FButtonStyle()
		.SetNormal(Background)
		.SetHovered(Highlight)
		.SetPressed(Highlight)
		.SetNormalPadding(FMargin(8.0f, 5.0f))
		.SetPressedPadding(FMargin(8.0f, 5.0f));
}

UTexture2D* MVInventoryPageCreateSlotHighlight()
{
	constexpr int32 GradientHeight = 32;
	FColor Pixels[GradientHeight];
	for (int32 Row = 0; Row < GradientHeight; ++Row)
	{
		const float Position = static_cast<float>(Row) / (GradientHeight - 1);
		const uint8 Alpha = static_cast<uint8>(FMath::RoundToInt(220.0f * Position * Position));
		Pixels[Row] = FColor(255, 255, 255, Alpha);
	}
	return UTexture2D::CreateTransient(1, GradientHeight, PF_B8G8R8A8, NAME_None,
		TConstArrayView64<uint8>(reinterpret_cast<const uint8*>(Pixels), sizeof(Pixels)));
}
}

void UMVInventoryButton::SetClickAction(FSimpleDelegate InAction)
{
	ClickAction = MoveTemp(InAction);
	OnClicked.AddUniqueDynamic(this, &ThisClass::RunClickAction);
}

void UMVInventoryButton::SetItemHighlight(UImage& Highlight)
{
	ItemHighlight = &Highlight;
}

void UMVInventoryButton::SetSelected(const bool bSelected)
{
	if (ItemHighlight)
	{
		ItemHighlight->SetColorAndOpacity(bSelected
			? FLinearColor(1.0f, 0.85f, 0.61f, 1.0f)
			: FLinearColor(0.81f, 0.73f, 0.61f, 0.50f));
		return;
	}
	SetBackgroundColor(bSelected ? FLinearColor(1.0f, 0.83f, 0.56f) : FLinearColor(0.64f, 0.64f, 0.64f));
}

void UMVInventoryButton::RunClickAction()
{
	ClickAction.ExecuteIfBound();
}

void UMVInventoryPage::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ItemGrid = WidgetTree->FindWidget<UUniformGridPanel>(TEXT("UniformInventoryGrid"));
	ItemScroll = WidgetTree->FindWidget<UScrollBox>(TEXT("InventoryScrollBox"));
	CategoryHeading = WidgetTree->FindWidget<UTextBlock>(TEXT("CategoryNameText"));
	ItemName = WidgetTree->FindWidget<UTextBlock>(TEXT("ItemNameText_Details"));
	ItemType = WidgetTree->FindWidget<UTextBlock>(TEXT("ItemCategoryText"));
	Description = WidgetTree->FindWidget<UTextBlock>(TEXT("ItemDescription"));
	UHorizontalBox* Filters = WidgetTree->FindWidget<UHorizontalBox>(TEXT("CategoriesBox"));
	UCanvasPanel* DetailCanvas = WidgetTree->FindWidget<UCanvasPanel>(TEXT("Canvas"));
	UBorder* SlotArtwork = WidgetTree->FindWidget<UBorder>(TEXT("EmptySlot0_BackgroundBorder"));
	UImage* TitleIcon = WidgetTree->FindWidget<UImage>(TEXT("TitleIcon"));
	if (!ensureMsgf(ItemGrid && ItemScroll && CategoryHeading && ItemName && ItemType && Description && Filters
		&& DetailCanvas && SlotArtwork && TitleIcon, TEXT("Inventory page layout is missing a required widget.")))
	{
		return;
	}

	MVInventoryPageSetItemFont(*ItemName);
	MVInventoryPageSetItemFont(*Description);
	if (UOverlay* Information = Cast<UOverlay>(Description->GetParent()))
	{
		DescriptionScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("InventoryDescriptionScroll"));
		const int32 DescriptionIndex = Information->GetChildIndex(Description);
		auto* ScrollSlot = CastChecked<UOverlaySlot>(Information->InsertChildAt(DescriptionIndex, DescriptionScroll, Description->Slot));
		ScrollSlot->SetHorizontalAlignment(HAlign_Fill);
		ScrollSlot->SetVerticalAlignment(VAlign_Fill);
		DescriptionScroll->AddChild(Description);
		Description->SetWrapTextAt(0.0f);
		Description->SetAutoWrapText(true);
	}

	FallbackIcon = Cast<UTexture2D>(TitleIcon->GetBrush().GetResourceObject());
	SlotHighlightTexture = MVInventoryPageCreateSlotHighlight();
	const FButtonStyle ButtonStyle = MVInventoryPageButtonStyle(SlotArtwork->Background);
	Filters->ClearChildren();
	for (const auto& Filter : MVInventoryPageCategories)
	{
		auto* Button = WidgetTree->ConstructWidget<UMVInventoryButton>();
		Button->SetStyle(ButtonStyle);
		Button->SetSelected(Filter == Category);
		Button->SetClickAction(FSimpleDelegate::CreateWeakLambda(this, [this, Filter]() { ChooseCategory(Filter); }));
		auto* Label = WidgetTree->ConstructWidget<UTextBlock>();
		Label->SetText(MVInventoryPageCategoryName(Filter));
		Label->SetFont(CategoryHeading->GetFont());
		Label->SetVisibility(ESlateVisibility::HitTestInvisible);
		Button->AddChild(Label);
		Filters->AddChildToHorizontalBox(Button)->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
		CategoryButtons.Add(Button);
	}

	ItemQuantity = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InventorySelectedQuantity"));
	ItemQuantity->SetFont(ItemType->GetFont());
	auto* QuantitySlot = DetailCanvas->AddChildToCanvas(ItemQuantity);
	QuantitySlot->SetPosition(FVector2D(40.0f, 103.0f));
	QuantitySlot->SetAutoSize(true);

	ItemIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("InventorySelectedIcon"));
	ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	auto* IconSlot = DetailCanvas->AddChildToCanvas(ItemIcon);
	IconSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	IconSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	IconSlot->SetPosition(FVector2D(-30.0f, 55.0f));
	IconSlot->SetSize(FVector2D(124.0f, 124.0f));

	ItemGrid->SetMinDesiredSlotWidth(98.0f);
	ItemGrid->SetMinDesiredSlotHeight(104.0f);
	ItemGrid->SetSlotPadding(FMargin(3.0f));
	ConfigureResponsiveLayout();
	ItemScroll->SetNavigationDestination(EDescendantScrollDestination::IntoView);
	ItemScroll->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::InstantScroll);
	RefreshItems();
}

void UMVInventoryPage::ConfigureResponsiveLayout()
{
	UWidget* List = WidgetTree->FindWidget(TEXT("InventoryOverlay"));
	UWidget* Details = WidgetTree->FindWidget(TEXT("ItemInfoBoxSwitcher"));
	auto* PageContent = WidgetTree->FindWidget<USizeBox>(TEXT("VerticalSizeLimiter"));
	auto* ListSlot = List ? Cast<UHorizontalBoxSlot>(List->Slot) : nullptr;
	auto* DetailsSlot = Details ? Cast<UHorizontalBoxSlot>(Details->Slot) : nullptr;
	UWidget* Body = List ? List->GetParent() : nullptr;
	auto* BodySlot = Body ? Cast<UVerticalBoxSlot>(Body->Slot) : nullptr;
	auto* PageSlot = PageContent ? Cast<UOverlaySlot>(PageContent->Slot) : nullptr;
	if (!ensureMsgf(ListSlot && DetailsSlot && BodySlot && PageSlot,
		TEXT("Inventory page requires a two-column body and a content overlay slot.")))
	{
		return;
	}

	const FSlateChildSize Fill(ESlateSizeRule::Fill);
	ListSlot->SetSize(Fill);
	DetailsSlot->SetSize(Fill);
	BodySlot->SetSize(Fill);
	FMargin DetailsPadding = DetailsSlot->GetPadding();
	DetailsPadding.Left = MVInventoryDetailsGap;
	DetailsSlot->SetPadding(DetailsPadding);
	const auto* ListSizer = WidgetTree->FindWidget<USizeBox>(TEXT("InventorySizer"));
	auto* DetailsSizer = WidgetTree->FindWidget<USizeBox>(TEXT("ItemInfoSizer"));
	if (ListSizer && DetailsSizer)
	{
		const FMargin ListPadding = ListSlot->GetPadding();
		PageContent->SetWidthOverride(ListSizer->GetWidthOverride() + DetailsSizer->GetWidthOverride()
			+ ListPadding.Left + ListPadding.Right + DetailsPadding.Left + DetailsPadding.Right);
	}
	FMargin PagePadding = PageSlot->GetPadding();
	if (auto* Title = WidgetTree->FindWidget<USizeBox>(TEXT("TitleSizer")))
	{
		if (const auto* TitleSlot = Cast<UOverlaySlot>(Title->Slot))
		{
			PagePadding.Top = TitleSlot->GetPadding().Top + Title->GetHeightOverride();
		}
	}
	CategoryHeading->SetVisibility(ESlateVisibility::Collapsed);
	if (auto* CategorySizer = WidgetTree->FindWidget<USizeBox>(TEXT("CategorySizer")))
	{
		CategorySizer->SetHeightOverride(48.0f);
	}
	if (UWidget* Footer = WidgetTree->FindWidget(TEXT("ButtonsBox")))
	{
		if (auto* FooterSlot = Cast<UOverlaySlot>(Footer->Slot))
		{
			FMargin FooterPadding = FooterSlot->GetPadding();
			FooterPadding.Bottom = 18.0f;
			FooterSlot->SetPadding(FooterPadding);
			FooterSlot->SetVerticalAlignment(VAlign_Bottom);
			PagePadding.Bottom = 60.0f;
		}
	}
	PageSlot->SetVerticalAlignment(VAlign_Fill);
	PageSlot->SetHorizontalAlignment(HAlign_Left);
	PageSlot->SetPadding(PagePadding);
	if (auto* DetailScale = WidgetTree->FindWidget<UScaleBox>(TEXT("InfoScaler")))
	{
		// 세로 여백 때문에 작은 화면의 설명 글자가 지나치게 작아지지 않도록 한다.
		if (DetailsSizer)
		{
			DetailsSizer->SetHeightOverride(520.0f);
		}
		DetailScale->SetStretch(EStretch::ScaleToFit);
		DetailScale->SetStretchDirection(EStretchDirection::DownOnly);
		if (auto* DetailScaleSlot = Cast<UScaleBoxSlot>(DetailScale->GetContentSlot()))
		{
			DetailScaleSlot->SetHorizontalAlignment(HAlign_Left);
			DetailScaleSlot->SetVerticalAlignment(VAlign_Top);
		}
	}

	// 목록의 스크롤 영역은 탭 아래에 남은 높이만 사용한다.
	if (auto* ScrollSlot = Cast<UVerticalBoxSlot>(ItemScroll->Slot))
	{
		ScrollSlot->SetSize(Fill);
	}
	UPanelWidget* ScrollContainer = ItemScroll->GetParent();
	if (UPanelWidget* ScrollOverlay = ScrollContainer ? ScrollContainer->GetParent() : nullptr)
	{
		if (auto* ScrollOverlaySlot = Cast<UVerticalBoxSlot>(ScrollOverlay->Slot))
		{
			ScrollOverlaySlot->SetSize(Fill);
		}
	}
}

void UMVInventoryPage::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshNavigation();
}

void UMVInventoryPage::SetInventory(UMVInventoryComponent* Inventory)
{
	if (ObservedInventory.Get() == Inventory && !ObservedInventory.IsStale())
	{
		return;
	}
	if (UMVInventoryComponent* Previous = ObservedInventory.Get())
	{
		Previous->OnInventoryChanged.RemoveDynamic(this, &ThisClass::HandleInventoryChanged);
	}
	if (UMVTableManager* Tables = UMVTableManager::Get(this))
	{
		Tables->OnInventoryItemDefinitionsReloaded.RemoveAll(this);
		if (Inventory)
		{
			Tables->OnInventoryItemDefinitionsReloaded.AddUObject(this, &ThisClass::RefreshItems);
		}
	}
	ObservedInventory = Inventory;
	Category.Reset();
	if (ItemScroll)
	{
		ItemScroll->ScrollToStart();
	}
	SelectedItem = FGameplayTag();
	if (Inventory)
	{
		Inventory->OnInventoryChanged.AddUniqueDynamic(this, &ThisClass::HandleInventoryChanged);
	}
	RefreshItems();
}

void UMVInventoryPage::NativeDestruct()
{
	SetInventory(nullptr);
	Super::NativeDestruct();
}

UWidget* UMVInventoryPage::GetPreferredFocus() const
{
	if (const auto* Button = ItemButtons.Find(SelectedItem))
	{
		return Button->Get();
	}
	return GetActiveCategoryButton();
}

UWidget* UMVInventoryPage::GetActiveCategoryButton() const
{
	const int32 Index = MakeArrayView(MVInventoryPageCategories).Find(Category);
	return CategoryButtons.IsValidIndex(Index) ? CategoryButtons[Index].Get() : nullptr;
}

void UMVInventoryPage::ConfirmFocusedChoice()
{
	APlayerController* Player = GetOwningPlayer();
	if (!Player)
	{
		return;
	}
	for (UMVInventoryButton* Button : CategoryButtons)
	{
		if (Button->GetIsEnabled() && Button->HasUserFocus(Player))
		{
			Button->OnClicked.Broadcast();
			return;
		}
	}
	for (const auto& Pair : ItemButtons)
	{
		if (Pair.Value->GetIsEnabled() && Pair.Value->HasUserFocus(Player))
		{
			Pair.Value->OnClicked.Broadcast();
			return;
		}
	}
}

void UMVInventoryPage::ChooseCategory(const TOptional<EMVInventoryGroupCategory> NewCategory)
{
	if (Category == NewCategory)
	{
		return;
	}
	Category = NewCategory;
	ItemScroll->ScrollToStart();
	RefreshItems();
}

void UMVInventoryPage::ChooseItem(const FGameplayTag ItemTag)
{
	if (SelectedItem != ItemTag && ItemButtons.Contains(ItemTag))
	{
		SelectedItem = ItemTag;
		RefreshDetails();
	}
}

void UMVInventoryPage::HandleInventoryChanged(FGameplayTag ItemTag)
{
	RefreshItems();
}

void UMVInventoryPage::AddGridCell(UWidget& Cell, const int32 Index)
{
	auto* GridSlot = ItemGrid->AddChildToUniformGrid(&Cell, Index / MVInventoryPageColumns, Index % MVInventoryPageColumns);
	GridSlot->SetHorizontalAlignment(HAlign_Fill);
	GridSlot->SetVerticalAlignment(VAlign_Fill);
}

void UMVInventoryPage::RefreshNavigation()
{
	if (!ItemGrid || CategoryButtons.IsEmpty())
	{
		return;
	}

	const auto SetDirection = [](UWidget& From, const EUINavigation Direction, UWidget* To)
	{
		if (To)
		{
			From.SetNavigationRuleExplicit(Direction, To);
		}
		else
		{
			From.SetNavigationRuleBase(Direction, EUINavigationRule::Stop);
		}
	};
	const auto GridButton = [this](const int32 RowIndex, const int32 ColumnIndex) -> UMVInventoryButton*
	{
		if (RowIndex < 0 || ColumnIndex < 0 || ColumnIndex >= MVInventoryPageColumns)
		{
			return nullptr;
		}
		return Cast<UMVInventoryButton>(ItemGrid->GetChildAt(RowIndex * MVInventoryPageColumns + ColumnIndex));
	};
	const auto ClosestInRow = [&GridButton](const int32 RowIndex, const int32 ColumnIndex) -> UMVInventoryButton*
	{
		// 각 줄은 왼쪽부터 채워지므로, 같은 열이 비면 왼쪽에 가장 가까운 칸이 있다.
		for (int32 Column = ColumnIndex; Column >= 0; --Column)
		{
			if (auto* Button = GridButton(RowIndex, Column))
			{
				return Button;
			}
		}
		return nullptr;
	};

	UWidget* ActiveTab = GetActiveCategoryButton();
	UWidget* FirstItem = GridButton(0, 0);
	for (int32 Index = 0; Index < CategoryButtons.Num(); ++Index)
	{
		UMVInventoryButton& Tab = *CategoryButtons[Index];
		SetDirection(Tab, EUINavigation::Left, Index > 0 ? CategoryButtons[Index - 1].Get() : nullptr);
		SetDirection(Tab, EUINavigation::Right, Index + 1 < CategoryButtons.Num() ? CategoryButtons[Index + 1].Get() : nullptr);
		SetDirection(Tab, EUINavigation::Up, nullptr);
		SetDirection(Tab, EUINavigation::Down, FirstItem);
	}

	for (int32 Index = 0; Index < ItemGrid->GetChildrenCount(); ++Index)
	{
		const int32 RowIndex = Index / MVInventoryPageColumns;
		const int32 ColumnIndex = Index % MVInventoryPageColumns;
		if (UMVInventoryButton* Button = GridButton(RowIndex, ColumnIndex))
		{
			SetDirection(*Button, EUINavigation::Left, GridButton(RowIndex, ColumnIndex - 1));
			SetDirection(*Button, EUINavigation::Right, GridButton(RowIndex, ColumnIndex + 1));
			SetDirection(*Button, EUINavigation::Up, RowIndex == 0 ? ActiveTab : ClosestInRow(RowIndex - 1, ColumnIndex));
			SetDirection(*Button, EUINavigation::Down, ClosestInRow(RowIndex + 1, ColumnIndex));
		}
	}
}

void UMVInventoryPage::RefreshItems()
{
	if (!ItemGrid || !ItemIcon || CategoryButtons.IsEmpty())
	{
		return;
	}
	APlayerController* Player = GetOwningPlayer();
	bool bFocusWasInGrid = false;
	for (const auto& Pair : ItemButtons)
	{
		bFocusWasInGrid |= Player && Pair.Value->HasUserFocus(Player);
	}

	for (int32 Index = 0; Index < CategoryButtons.Num(); ++Index)
	{
		const bool bActive = MVInventoryPageCategories[Index] == Category;
		CategoryButtons[Index]->SetSelected(bActive);
		CategoryButtons[Index]->SetColorAndOpacity(bActive ? FLinearColor(1.0f, 0.87f, 0.63f) : FLinearColor::White);
	}
	ItemGrid->ClearChildren();
	ItemButtons.Reset();

	const UMVTableManager* Tables = UMVTableManager::Get(this);
	const UMVInventoryComponent* Inventory = ObservedInventory.Get();
	TArray<FMVInventoryPageEntry> VisibleItems;
	if (Tables && Inventory)
	{
		VisibleItems.Reserve(Inventory->GetItems().Num());
		for (const auto& [Tag, Quantity] : Inventory->GetItems())
		{
			const auto* Definition = Tables->FindInventoryItemDefinition(Tag);
			if (!Definition || (Category.IsSet() && Category.GetValue() != Definition->InventoryGroupCategory))
			{
				continue;
			}
			if (const auto* Row = Definition->Row.GetRow<FMVItemTableRow>(TEXT("MVInventoryPage")))
			{
				VisibleItems.Add({Tag, Quantity, Definition, Row});
			}
		}
		VisibleItems.Sort([](const FMVInventoryPageEntry& A, const FMVInventoryPageEntry& B)
		{
			const auto& Left = *A.Definition;
			const auto& Right = *B.Definition;
			if (Left.InventoryGroupCategory != Right.InventoryGroupCategory)
			{
				return Left.InventoryGroupCategory < Right.InventoryGroupCategory;
			}
			if (Left.InventoryGroupSortOrder != Right.InventoryGroupSortOrder)
			{
				return Left.InventoryGroupSortOrder < Right.InventoryGroupSortOrder;
			}
			if (Left.InventoryGroupId != Right.InventoryGroupId)
			{
				return Left.InventoryGroupId.LexicalLess(Right.InventoryGroupId);
			}
			if (Left.InventorySortOrder != Right.InventorySortOrder)
			{
				return Left.InventorySortOrder < Right.InventorySortOrder;
			}
			return A.Tag.GetTagName().LexicalLess(B.Tag.GetTagName());
		});
	}

	FButtonStyle ItemStyle = CategoryButtons[0]->GetStyle();
	const FSlateRoundedBoxBrush SlotBrush(FLinearColor(0.06f, 0.05f, 0.04f, 0.36f), 3.0f);
	ItemStyle.SetNormal(SlotBrush).SetHovered(SlotBrush).SetPressed(SlotBrush)
		.SetNormalPadding(FMargin(1.0f, 2.0f)).SetPressedPadding(FMargin(1.0f, 2.0f));
	int32 CellIndex = 0;
	const auto FillEmptyCells = [this, &SlotBrush, &CellIndex](const int32 EndIndex)
	{
		while (CellIndex < EndIndex)
		{
			auto* EmptyCell = WidgetTree->ConstructWidget<UImage>();
			EmptyCell->SetBrush(SlotBrush);
			EmptyCell->SetVisibility(ESlateVisibility::HitTestInvisible);
			AddGridCell(*EmptyCell, CellIndex++);
		}
	};

	const FMVInventoryItemDefinition* Previous = nullptr;
	for (const FMVInventoryPageEntry& Entry : VisibleItems)
	{
		const auto* Definition = Entry.Definition;
		if (Previous && (Previous->InventoryGroupId != Definition->InventoryGroupId
			|| Previous->InventoryGroupCategory != Definition->InventoryGroupCategory))
		{
			FillEmptyCells(AlignArbitrary(CellIndex, MVInventoryPageColumns));
		}
		Previous = Definition;
		const FGameplayTag Tag = Entry.Tag;
		auto* Button = WidgetTree->ConstructWidget<UMVInventoryButton>();
		Button->SetStyle(ItemStyle);
		Button->SetToolTipText(MVInventoryPageDisplayName(*Entry.Row));
		Button->SetClickAction(FSimpleDelegate::CreateWeakLambda(this, [this, Tag]() { ChooseItem(Tag); }));
		Button->OnReceivedFocus = FSimpleDelegate::CreateUObject(this, &ThisClass::ChooseItem, Tag);

		auto* CellOverlay = WidgetTree->ConstructWidget<UOverlay>();
		auto* Highlight = WidgetTree->ConstructWidget<UImage>();
		Highlight->SetBrushFromTexture(SlotHighlightTexture);
		auto* HighlightSizer = WidgetTree->ConstructWidget<USizeBox>();
		HighlightSizer->SetHeightOverride(MVInventoryHighlightHeight);
		HighlightSizer->SetVisibility(ESlateVisibility::HitTestInvisible);
		HighlightSizer->SetContent(Highlight);
		auto* HighlightSlot = CellOverlay->AddChildToOverlay(HighlightSizer);
		HighlightSlot->SetHorizontalAlignment(HAlign_Fill);
		HighlightSlot->SetVerticalAlignment(VAlign_Bottom);
		Button->SetItemHighlight(*Highlight);

		auto* Contents = WidgetTree->ConstructWidget<UVerticalBox>();
		auto* StackSlot = CellOverlay->AddChildToOverlay(Contents);
		StackSlot->SetHorizontalAlignment(HAlign_Fill);
		StackSlot->SetVerticalAlignment(VAlign_Fill);
		auto* Icon = WidgetTree->ConstructWidget<UImage>();
		Icon->SetBrushFromTexture(MVInventoryPageResolveIcon(*Entry.Row, FallbackIcon));
		FSlateBrush IconBrush = Icon->GetBrush();
		IconBrush.SetImageSize(FVector2D(72.0f, 72.0f));
		Icon->SetBrush(IconBrush);
		auto* IconScale = WidgetTree->ConstructWidget<UScaleBox>();
		IconScale->SetStretch(EStretch::ScaleToFit);
		IconScale->SetStretchDirection(EStretchDirection::DownOnly);
		IconScale->SetVisibility(ESlateVisibility::HitTestInvisible);
		IconScale->SetContent(Icon);
		Contents->AddChildToVerticalBox(IconScale)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		auto* Count = WidgetTree->ConstructWidget<UTextBlock>();
		FSlateFontInfo CountFont = CategoryHeading->GetFont();
		CountFont.Size = 16.0f;
		Count->SetFont(CountFont);
		Count->SetText(FText::AsNumber(Entry.Quantity));
		Count->SetVisibility(ESlateVisibility::HitTestInvisible);
		auto* CountSlot = Contents->AddChildToVerticalBox(Count);
		CountSlot->SetHorizontalAlignment(HAlign_Right);
		auto* ContentSlot = CastChecked<UButtonSlot>(Button->AddChild(CellOverlay));
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
		ContentSlot->SetVerticalAlignment(VAlign_Fill);

		AddGridCell(*Button, CellIndex++);
		ItemButtons.Add(Tag, Button);
	}
	FillEmptyCells(FMath::Max(MVInventoryPageColumns, AlignArbitrary(CellIndex, MVInventoryPageColumns)));
	RefreshNavigation();

	if (!ItemButtons.Contains(SelectedItem))
	{
		SelectedItem = VisibleItems.IsEmpty() ? FGameplayTag() : VisibleItems[0].Tag;
	}
	RefreshDetails();
	if (bFocusWasInGrid && Inventory)
	{
		if (UWidget* Focus = GetPreferredFocus())
		{
			Focus->SetUserFocus(Player);
		}
	}
}

void UMVInventoryPage::RefreshDetails()
{
	for (const auto& Pair : ItemButtons)
	{
		Pair.Value->SetSelected(Pair.Key == SelectedItem);
	}

	const UMVTableManager* Tables = UMVTableManager::Get(this);
	const auto* Definition = Tables ? Tables->FindInventoryItemDefinition(SelectedItem) : nullptr;
	const auto* Row = Definition ? Definition->Row.GetRow<FMVItemTableRow>(TEXT("MVInventoryPage")) : nullptr;
	ItemName->SetText(Row ? MVInventoryPageDisplayName(*Row) : LOCTEXT("Empty", "No items in this category"));
	ItemType->SetText(Row ? UEnum::GetDisplayValueAsText(Row->ItemType) : FText::GetEmpty());
	const FText DetailText = Row && Row->Description.IsEmpty()
		? LOCTEXT("NoDescription", "No description available.")
		: Row ? Row->Description : FText::GetEmpty();
	if (DescriptionScroll && !Description->GetText().EqualTo(DetailText))
	{
		DescriptionScroll->ScrollToStart();
	}
	Description->SetText(DetailText);
	const UMVInventoryComponent* Inventory = ObservedInventory.Get();
	ItemQuantity->SetText(Row && Inventory
		? FText::Format(LOCTEXT("Quantity", "Held: {0}"), FText::AsNumber(Inventory->GetItemCount(SelectedItem)))
		: FText::GetEmpty());
	ItemIcon->SetBrushFromTexture(Row ? MVInventoryPageResolveIcon(*Row, FallbackIcon) : nullptr);
	ItemIcon->SetVisibility(Row ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

#undef LOCTEXT_NAMESPACE
