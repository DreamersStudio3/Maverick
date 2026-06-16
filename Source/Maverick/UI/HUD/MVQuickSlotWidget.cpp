#include "UI/HUD/MVQuickSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

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

	SetIsEnabled(!InViewData.bLocked);
}
