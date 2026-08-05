#include "UI/HUD/MVSkillSlotWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "UI/HUD/MVCooldownOverlayWidget.h"

void UMVSkillSlotWidget::SetIconStack(const TArray<UTexture2D*>& InIconStack)
{
	IconStack.Reset(InIconStack.Num());
	for (UTexture2D* Icon : InIconStack)
	{
		IconStack.Add(Icon);
	}

	ActiveStackIndex = FMath::Clamp(ActiveStackIndex, 0, FMath::Max(0, IconStack.Num() - 1));
	ApplyIcon();
}

void UMVSkillSlotWidget::SetHotKeyText(FText InHotKeyText)
{
	HotKeyText = MoveTemp(InHotKeyText);
	ApplyHotKeyText();
}

void UMVSkillSlotWidget::SetRuntimeState(
	const int32 InActiveStackIndex,
	const float CooldownRemaining,
	const float CooldownDuration,
	const bool bInAvailable)
{
	const int32 PreviousStackIndex = ActiveStackIndex;
	const bool bAvailabilityChanged = bAvailable != bInAvailable;
	ActiveStackIndex = FMath::Clamp(InActiveStackIndex, 0, FMath::Max(0, IconStack.Num() - 1));
	bAvailable = bInAvailable;

	if (PreviousStackIndex != ActiveStackIndex)
	{
		ApplyIcon();
	}

	float CooldownPercent = 0.0f;
	if (CooldownOverlay)
	{
		CooldownPercent = CooldownDuration > KINDA_SMALL_NUMBER
			? FMath::Clamp(CooldownRemaining / CooldownDuration, 0.0f, 1.0f)
			: 0.0f;
		CooldownOverlay->SetCooldownPercent(CooldownPercent);
	}

	if (bAvailabilityChanged)
	{
		SetRenderOpacity(bAvailable ? 1.0f : 0.42f);
	}
}

void UMVSkillSlotWidget::SetSlotSize(const float InSlotSize)
{
	DesiredSlotSize = FMath::Max(1.0f, InSlotSize);
	ApplySlotSize();
}

void UMVSkillSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BuildNativeWidgetTree())
	{
		ApplyNativeFrameStyle();
	}

	ApplySlotSize();
	ApplyIcon();
	ApplyHotKeyText();
	if (CooldownOverlay)
	{
		CooldownOverlay->SetOverlayColor(CooldownColor);
	}
}

void UMVSkillSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplySlotSize();
	ApplyIcon();
	ApplyHotKeyText();
	if (CooldownOverlay)
	{
		CooldownOverlay->SetOverlayColor(CooldownColor);
	}
}

bool UMVSkillSlotWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return false;
	}

	SlotSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SlotSizeBox"));
	SlotFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SlotFrame"));
	UOverlay* SlotOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("SlotOverlay"));
	SkillIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SkillIcon"));
	CooldownOverlay = WidgetTree->ConstructWidget<UMVCooldownOverlayWidget>(
		UMVCooldownOverlayWidget::StaticClass(),
		TEXT("CooldownOverlay"));
	HotKeyTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HotKeyTextBlock"));

	WidgetTree->RootWidget = SlotSizeBox;
	SlotSizeBox->AddChild(SlotFrame);
	SlotFrame->SetPadding(FMargin(3.0f));
	SlotFrame->AddChild(SlotOverlay);

	if (UOverlaySlot* IconSlot = SlotOverlay->AddChildToOverlay(SkillIcon))
	{
		IconSlot->SetHorizontalAlignment(HAlign_Fill);
		IconSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UOverlaySlot* CooldownSlot = SlotOverlay->AddChildToOverlay(CooldownOverlay))
	{
		CooldownSlot->SetHorizontalAlignment(HAlign_Fill);
		CooldownSlot->SetVerticalAlignment(VAlign_Fill);
	}
	CooldownOverlay->SetClipping(EWidgetClipping::ClipToBounds);

	if (UOverlaySlot* HotKeySlot = SlotOverlay->AddChildToOverlay(HotKeyTextBlock))
	{
		HotKeySlot->SetHorizontalAlignment(HAlign_Left);
		HotKeySlot->SetVerticalAlignment(VAlign_Bottom);
		HotKeySlot->SetPadding(FMargin(4.0f, 0.0f, 0.0f, 2.0f));
	}

	FSlateFontInfo HotKeyFont = HotKeyTextBlock->GetFont();
	HotKeyFont.Size = 14;
	HotKeyTextBlock->SetFont(HotKeyFont);
	HotKeyTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.95f, 1.0f, 1.0f)));
	HotKeyTextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
	HotKeyTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));

	return true;
}

void UMVSkillSlotWidget::ApplyNativeFrameStyle()
{
	if (!SlotFrame)
	{
		return;
	}

	SlotFrame->SetBrush(FSlateRoundedBoxBrush(
		SlotBackgroundColor,
		CornerRadius,
		SlotOutlineColor,
		SlotOutlineWidth,
		FVector2D(DesiredSlotSize, DesiredSlotSize)));
}

void UMVSkillSlotWidget::ApplyIcon()
{
	if (!SkillIcon)
	{
		return;
	}

	UTexture2D* ActiveIcon = IconStack.IsValidIndex(ActiveStackIndex)
		? IconStack[ActiveStackIndex].Get()
		: nullptr;
	if (ActiveIcon)
	{
		SkillIcon->SetBrushFromTexture(ActiveIcon, true);
		SkillIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		SkillIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMVSkillSlotWidget::ApplyHotKeyText()
{
	if (HotKeyTextBlock)
	{
		HotKeyTextBlock->SetText(HotKeyText);
	}
}

void UMVSkillSlotWidget::ApplySlotSize()
{
	if (SlotSizeBox)
	{
		SlotSizeBox->SetWidthOverride(DesiredSlotSize);
		SlotSizeBox->SetHeightOverride(DesiredSlotSize);
	}
}
