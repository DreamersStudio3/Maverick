#include "UI/Popup/MVInteractionChoicePopup.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

UMVChoiceEntryWidget::UMVChoiceEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVChoiceEntryWidget::SetEntryData(const FMVMenuEntryData& InEntryData)
{
	EntryData = InEntryData;
	SetIsEnabled(EntryData.bEnabled);
	if (EntryButton)
	{
		EntryButton->SetIsEnabled(EntryData.bEnabled);
	}
	RefreshEntry();
}

void UMVChoiceEntryWidget::NativeConstruct()
{
	CacheBoundEntryWidgets();
	if (!EntryButton)
	{
		BuildNativeEntryTree();
	}
	CacheBoundEntryWidgets();

	Super::NativeConstruct();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddUniqueDynamic(this, &UMVChoiceEntryWidget::HandleClicked);
	}
	RefreshEntry();
}

void UMVChoiceEntryWidget::NativeDestruct()
{
	if (EntryButton)
	{
		EntryButton->OnClicked.RemoveDynamic(this, &UMVChoiceEntryWidget::HandleClicked);
	}

	Super::NativeDestruct();
}

void UMVChoiceEntryWidget::CacheBoundEntryWidgets()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!EntryButton)
	{
		EntryButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("EntryButton")));
	}
	if (!LabelText)
	{
		LabelText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("LabelText")));
	}
}

void UMVChoiceEntryWidget::BuildNativeEntryTree()
{
	if (!WidgetTree)
	{
		return;
	}

	EntryButton = WidgetTree->ConstructWidget<UButton>(
		UButton::StaticClass(),
		TEXT("EntryButton"));
	LabelText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(),
		TEXT("LabelText"));
	if (!EntryButton || !LabelText)
	{
		return;
	}

	WidgetTree->RootWidget = EntryButton;

	FButtonStyle ButtonStyle = EntryButton->GetStyle();
	ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(0.10f, 0.095f, 0.08f, 0.82f));
	ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.24f, 0.22f, 0.17f, 0.92f));
	ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.32f, 0.29f, 0.20f, 0.96f));
	ButtonStyle.Disabled.TintColor = FSlateColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.55f));
	EntryButton->SetStyle(ButtonStyle);
	EntryButton->SetContent(LabelText);
}

void UMVChoiceEntryWidget::RefreshEntry()
{
	if (LabelText)
	{
		LabelText->SetText(ResolveEntryLabel());
		LabelText->SetJustification(ETextJustify::Center);
		LabelText->SetAutoWrapText(true);
		LabelText->SetColorAndOpacity(EntryData.bEnabled
			? FSlateColor(FLinearColor(0.88f, 0.84f, 0.75f, 1.0f))
			: FSlateColor(FLinearColor(0.48f, 0.46f, 0.42f, 1.0f)));
	}
}

FText UMVChoiceEntryWidget::ResolveEntryLabel() const
{
	return EntryData.Label;
}

void UMVChoiceEntryWidget::HandleClicked()
{
	OnEntryWidgetClicked.Broadcast(this);
}

UMVChoicePopup::UMVChoicePopup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AutoDismissSeconds = 0.0f;
	EntryWidgetClass = UMVChoiceEntryWidget::StaticClass();
}

void UMVChoicePopup::SetChoiceData(
	const FMVInteractionChoiceData& InChoiceData,
	UObject* InSourceObject)
{
	ChoiceData = InChoiceData;
	SourceObject = InSourceObject;
	RefreshChoice();
}

void UMVChoicePopup::RefreshChoice()
{
	UVerticalBox* EntryContainer = ResolveChoiceBox();
	if (!EntryContainer)
	{
		BuildNativeChoiceTree();
		EntryContainer = ResolveChoiceBox();
		if (!EntryContainer)
		{
			SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}

	const bool bHasChoices = !ChoiceData.Choices.IsEmpty();
	SetVisibility(bHasChoices ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (WidgetTree && WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget->SetVisibility(bHasChoices
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	}

	if (PromptText)
	{
		PromptText->SetText(ChoiceData.PromptText);
		PromptText->SetVisibility(ChoiceData.PromptText.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible);
	}

	EntryContainer->ClearChildren();
	if (!bHasChoices)
	{
		return;
	}

	int32 ChoiceIndex = 0;
	for (const FMVInteractionChoiceEntryData& Choice : ChoiceData.Choices)
	{
		const FName EntryName = *FString::Printf(TEXT("InteractionChoiceEntry_%d"), ChoiceIndex++);
		TSubclassOf<UMVChoiceEntryWidget> ResolvedEntryWidgetClass = EntryWidgetClass;
		if (!ResolvedEntryWidgetClass)
		{
			ResolvedEntryWidgetClass = UMVChoiceEntryWidget::StaticClass();
		}
		UMVChoiceEntryWidget* EntryWidget =
			WidgetTree->ConstructWidget<UMVChoiceEntryWidget>(
				ResolvedEntryWidgetClass,
				EntryName);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetEntryData(MakeEntryData(Choice));
		EntryWidget->OnEntryWidgetClicked.AddUniqueDynamic(
			this,
			&UMVChoicePopup::HandleChoiceEntryClicked);

		if (UVerticalBoxSlot* EntrySlot = EntryContainer->AddChildToVerticalBox(EntryWidget))
		{
			EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}
	}
}

void UMVChoicePopup::NativeConstruct()
{
	CacheBoundChoiceWidgets();
	if (!ResolveChoiceBox())
	{
		BuildNativeChoiceTree();
	}
	CacheBoundChoiceWidgets();

	Super::NativeConstruct();
	RefreshChoice();
}

void UMVChoicePopup::CacheBoundChoiceWidgets()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!PromptText)
	{
		PromptText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("PromptText")));
		if (!PromptText)
		{
			PromptText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("InteractionChoicePrompt")));
		}
	}

	if (!ChoiceBox)
	{
		ChoiceBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("ChoiceBox")));
		if (!ChoiceBox)
		{
			ChoiceBox = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("InteractionChoiceEntryBox")));
		}
	}

	if (!EntryList)
	{
		EntryList = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("EntryList")));
	}
}

void UMVChoicePopup::BuildNativeChoiceTree()
{
	if (!WidgetTree)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(),
		TEXT("InteractionChoiceRoot"));
	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(),
		TEXT("InteractionChoicePanel"));
	UVerticalBox* PanelBox = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(),
		TEXT("InteractionChoicePanelBox"));
	PromptText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(),
		TEXT("InteractionChoicePrompt"));
	ChoiceBox = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(),
		TEXT("InteractionChoiceEntryBox"));
	EntryList = ChoiceBox;

	if (!RootCanvas || !PanelBorder || !PanelBox || !PromptText || !ChoiceBox)
	{
		return;
	}

	WidgetTree->RootWidget = RootCanvas;
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PanelBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PanelBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ChoiceBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	PanelBorder->SetPadding(FMargin(18.0f, 14.0f));
	PanelBorder->SetBrushColor(FLinearColor(0.015f, 0.014f, 0.012f, 0.84f));
	PanelBorder->SetContent(PanelBox);

	PromptText->SetText(ChoiceData.PromptText);
	PromptText->SetJustification(ETextJustify::Center);
	PromptText->SetAutoWrapText(true);
	PromptText->SetColorAndOpacity(FSlateColor(FLinearColor(0.74f, 0.70f, 0.62f, 1.0f)));

	if (UVerticalBoxSlot* PromptSlot = PanelBox->AddChildToVerticalBox(PromptText))
	{
		PromptSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}
	PanelBox->AddChildToVerticalBox(ChoiceBox);

	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBorder))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		PanelSlot->SetPosition(FVector2D(0.0f, -72.0f));
		PanelSlot->SetSize(FVector2D(760.0f, 220.0f));
	}
}

UVerticalBox* UMVChoicePopup::ResolveChoiceBox()
{
	CacheBoundChoiceWidgets();
	return ChoiceBox ? ChoiceBox.Get() : EntryList.Get();
}

FMVMenuEntryData UMVChoicePopup::MakeEntryData(
	const FMVInteractionChoiceEntryData& Choice) const
{
	FMVMenuEntryData EntryData;
	EntryData.EntryId = Choice.ChoiceId;
	EntryData.Label = Choice.Label;
	EntryData.bEnabled = Choice.bEnabled;
	return EntryData;
}

void UMVChoicePopup::HandleChoiceEntryClicked(UMVChoiceEntryWidget* EntryWidget)
{
	if (!EntryWidget)
	{
		return;
	}

	const FMVMenuEntryData& EntryData = EntryWidget->GetEntryData();
	if (!EntryData.bEnabled)
	{
		return;
	}

	OnChoiceEntrySelected.Broadcast(SourceObject, EntryData);
	ClosePopup();
}
