#include "UI/HUD/MVQuickSlotWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UMVQuickSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	auto* Size = WidgetTree->ConstructWidget<USizeBox>();
	Size->SetWidthOverride(64.0f);
	Size->SetHeightOverride(64.0f);
	Size->SetVisibility(ESlateVisibility::HitTestInvisible);
	WidgetTree->RootWidget = Size;

	auto* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.7f));
	Background->SetPadding(FMargin(5.0f));
	Size->SetContent(Background);

	auto* Stack = WidgetTree->ConstructWidget<UOverlay>();
	Background->SetContent(Stack);

	SlotIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SlotIcon"));
	SlotIcon->SetDesiredSizeOverride(FVector2D(40.0f, 40.0f));
	auto* IconSlot = Stack->AddChildToOverlay(SlotIcon);
	IconSlot->SetHorizontalAlignment(HAlign_Center);
	IconSlot->SetVerticalAlignment(VAlign_Center);

	HotKeyTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HotKeyTextBlock"));
	auto* HotKeySlot = Stack->AddChildToOverlay(HotKeyTextBlock);
	HotKeySlot->SetHorizontalAlignment(HAlign_Left);
	HotKeySlot->SetVerticalAlignment(VAlign_Top);

	CountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountText"));
	auto* QuantitySlot = Stack->AddChildToOverlay(CountText);
	QuantitySlot->SetHorizontalAlignment(HAlign_Right);
	QuantitySlot->SetVerticalAlignment(VAlign_Bottom);
}

void UMVQuickSlotWidget::SetViewData(const FMVQuickSlotViewData& InViewData)
{
	CurrentViewData = InViewData;
	ApplyViewData(CurrentViewData);
}

void UMVQuickSlotWidget::UpdateCount(int32 NewCount)
{
	CurrentViewData.Count = NewCount;
	ApplyViewData(CurrentViewData);
}

void UMVQuickSlotWidget::UpdateCountAndLocked(const int32 NewCount, const bool bLocked)
{
	CurrentViewData.Count = NewCount;
	CurrentViewData.bLocked = bLocked;
	ApplyViewData(CurrentViewData);
}

void UMVQuickSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		ApplyViewData(PreviewData);
	}
	else
	{
		ApplyViewData(CurrentViewData);
	}
}

void UMVQuickSlotWidget::ApplyViewData(const FMVQuickSlotViewData& InViewData)
{
	if (SlotIcon && InViewData.Icon)
	{
		SlotIcon->SetBrushFromTexture(InViewData.Icon);
	}

	if (CountText)
	{
		CountText->SetText(FText::AsNumber(InViewData.Count));
	}

	if (HotKeyTextBlock)
	{
		HotKeyTextBlock->SetText(InViewData.HotKeyText);
	}

	const bool bEffectiveLocked = InViewData.bLocked || InViewData.Count <= 0;
	SetIsEnabled(!bEffectiveLocked);
}
