#include "UI/HUD/MVStatusBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

namespace
{
// Mirrors the default stamina recovery speed without coupling HUD loss animation to runtime stat modifiers.
constexpr float DefaultRecentLossRecoveryUnitsPerSecond = 35.0f;
}

void UMVStatusBarWidget::SetLabel(FText InLabel)
{
	StatusLabel = InLabel;
	UpdateTextBlocks();
}

void UMVStatusBarWidget::SetFillColor(FLinearColor InFillColor)
{
	FillColor = InFillColor;
	UpdateBarStyle();
}

void UMVStatusBarWidget::SetBarSize(float InWidth, float InHeight)
{
	DesiredBarWidth = FMath::Max(0.0f, InWidth);
	DesiredBarHeight = FMath::Max(0.0f, InHeight);
	UpdateBarSize();
}

void UMVStatusBarWidget::SetLabelVisible(bool bInVisible)
{
	bShowLabelText = bInVisible;
	UpdateTextVisibility();
}

void UMVStatusBarWidget::SetValueVisible(bool bInVisible)
{
	bShowValueText = bInVisible;
	UpdateTextVisibility();
}

void UMVStatusBarWidget::SetRecentLossHoldLocked(bool)
{
}

void UMVStatusBarWidget::SetProgress(float CurrentValue, float MaxValue)
{
	const float PreviousPercent = CurrentDisplayPercent;
	TargetPercent = MaxValue > 0.0f ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;
	CurrentDisplayPercent = TargetPercent;

	if (Bar)
	{
		Bar->SetPercent(CurrentDisplayPercent);
	}

	const bool bShouldShowRecentLoss = bHasProgressValue
		&& CurrentValue < LastCurrentValue - RecentLossMinAmount
		&& CurrentDisplayPercent < PreviousPercent;
	if (RecentLossBar)
	{
		if (bShouldShowRecentLoss)
		{
			RecentLossDisplayPercent = FMath::Max(RecentLossDisplayPercent, PreviousPercent);
			RecentLossHoldRemaining = RecentLossHoldTime;
		}
		else if (CurrentDisplayPercent >= RecentLossDisplayPercent)
		{
			RecentLossDisplayPercent = CurrentDisplayPercent;
			RecentLossHoldRemaining = 0.0f;
		}

		RecentLossBar->SetPercent(FMath::Max(RecentLossDisplayPercent, CurrentDisplayPercent));
	}

	LastCurrentValue = CurrentValue;
	LastMaxValue = MaxValue;
	bHasProgressValue = true;
	UpdateTextBlocks();
}

void UMVStatusBarWidget::SetProgressImmediate(float CurrentValue, float MaxValue)
{
	TargetPercent = MaxValue > 0.0f ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;
	CurrentDisplayPercent = TargetPercent;
	RecentLossDisplayPercent = TargetPercent;
	RecentLossHoldRemaining = 0.0f;
	LastCurrentValue = CurrentValue;
	LastMaxValue = MaxValue;
	bHasProgressValue = true;
	UpdateTextBlocks();

	if (Bar)
	{
		Bar->SetPercent(CurrentDisplayPercent);
	}

	if (RecentLossBar)
	{
		RecentLossBar->SetPercent(RecentLossDisplayPercent);
	}
}

void UMVStatusBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildNativeWidgetTree();
	UpdateBarStyle();
	UpdateBarSize();
	UpdateTextBlocks();
	UpdateTextVisibility();
}

void UMVStatusBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Bar)
	{
		UpdateBarStyle();
		Bar->SetClipping(EWidgetClipping::ClipToBounds);
		Bar->SetPercent(CurrentDisplayPercent);
	}
	if (RecentLossBar)
	{
		RecentLossBar->SetClipping(EWidgetClipping::ClipToBounds);
		RecentLossBar->SetPercent(RecentLossDisplayPercent);
	}

	UpdateBarSize();
	UpdateTextBlocks();
	UpdateTextVisibility();
}

void UMVStatusBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!RecentLossBar)
	{
		return;
	}

	if (RecentLossHoldRemaining > 0.0f)
	{
		RecentLossHoldRemaining = FMath::Max(0.0f, RecentLossHoldRemaining - InDeltaTime);
		RecentLossBar->SetPercent(FMath::Max(RecentLossDisplayPercent, CurrentDisplayPercent));
		return;
	}

	if (RecentLossDisplayPercent > CurrentDisplayPercent)
	{
		const float MaxValue = FMath::Max(LastMaxValue, KINDA_SMALL_NUMBER);
		const float RecentLossDecreasePerSecond = DefaultRecentLossRecoveryUnitsPerSecond / MaxValue;
		RecentLossDisplayPercent = FMath::Max(
			CurrentDisplayPercent,
			RecentLossDisplayPercent - RecentLossDecreasePerSecond * InDeltaTime);
	}
	else
	{
		RecentLossDisplayPercent = CurrentDisplayPercent;
	}

	RecentLossBar->SetPercent(FMath::Max(RecentLossDisplayPercent, CurrentDisplayPercent));
}

void UMVStatusBarWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UHorizontalBox* RootBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StatusBarRoot"));
	LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LabelText"));
	Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("Bar"));
	RecentLossBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("RecentLossBar"));
	ValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ValueText"));
	BarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BarSizeBox"));
	UOverlay* BarOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("BarOverlay"));

	WidgetTree->RootWidget = RootBox;

	if (LabelText)
	{
		FSlateFontInfo Font = LabelText->GetFont();
		Font.Size = 13;
		LabelText->SetFont(Font);
		LabelText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f)));
	}

	if (ValueText)
	{
		FSlateFontInfo Font = ValueText->GetFont();
		Font.Size = 12;
		ValueText->SetFont(Font);
		ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.78f, 0.78f, 1.0f)));
	}

	if (BarSizeBox)
	{
		BarSizeBox->SetWidthOverride(DesiredBarWidth);
		BarSizeBox->SetHeightOverride(DesiredBarHeight);
		BarSizeBox->AddChild(BarOverlay);
	}

	if (BarOverlay)
	{
		if (UOverlaySlot* RecentLossSlot = BarOverlay->AddChildToOverlay(RecentLossBar))
		{
			RecentLossSlot->SetHorizontalAlignment(HAlign_Fill);
			RecentLossSlot->SetVerticalAlignment(VAlign_Fill);
		}

		if (UOverlaySlot* BarOverlaySlot = BarOverlay->AddChildToOverlay(Bar))
		{
			BarOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			BarOverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	if (UHorizontalBoxSlot* LabelSlot = RootBox->AddChildToHorizontalBox(LabelText))
	{
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	if (UHorizontalBoxSlot* BarSlot = RootBox->AddChildToHorizontalBox(BarSizeBox))
	{
		BarSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (UHorizontalBoxSlot* ValueSlot = RootBox->AddChildToHorizontalBox(ValueText))
	{
		ValueSlot->SetVerticalAlignment(VAlign_Center);
		ValueSlot->SetPadding(FMargin(10.0f, 0.0f, 0.0f, 0.0f));
	}
}

void UMVStatusBarWidget::UpdateBarStyle()
{
	if (Bar)
	{
		Bar->SetFillColorAndOpacity(FillColor);
	}

	if (RecentLossBar)
	{
		RecentLossBar->SetFillColorAndOpacity(RecentLossColor);
	}
}

void UMVStatusBarWidget::UpdateBarSize()
{
	if (BarSizeBox)
	{
		BarSizeBox->SetWidthOverride(DesiredBarWidth);
		BarSizeBox->SetHeightOverride(DesiredBarHeight);
	}
}

void UMVStatusBarWidget::UpdateTextBlocks()
{
	if (LabelText)
	{
		LabelText->SetText(StatusLabel);
	}

	if (ValueText)
	{
		const int32 CurrentInt = FMath::Max(0, FMath::RoundToInt(LastCurrentValue));
		const int32 MaxInt = FMath::Max(0, FMath::RoundToInt(LastMaxValue));
		ValueText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentInt, MaxInt)));
	}
}

void UMVStatusBarWidget::UpdateTextVisibility()
{
	if (LabelText)
	{
		LabelText->SetVisibility(bShowLabelText ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (ValueText)
	{
		ValueText->SetVisibility(bShowValueText ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
