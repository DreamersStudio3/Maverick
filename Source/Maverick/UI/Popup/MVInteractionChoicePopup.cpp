#include "UI/Popup/MVInteractionChoicePopup.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

UMVInteractionChoicePopupEntryButton::UMVInteractionChoicePopupEntryButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OnClicked.AddDynamic(this, &UMVInteractionChoicePopupEntryButton::HandleClicked);
}

void UMVInteractionChoicePopupEntryButton::SetEntryData(const FMVMenuEntryData& InEntryData)
{
	EntryData = InEntryData;
	SetIsEnabled(EntryData.bEnabled);
}

void UMVInteractionChoicePopupEntryButton::HandleClicked()
{
	OnEntryButtonClicked.Broadcast(this);
}

UMVInteractionChoicePopup::UMVInteractionChoicePopup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AutoDismissSeconds = 0.0f;
}

void UMVInteractionChoicePopup::SetChoiceData(
	const FMVInteractionChoiceData& InChoiceData,
	UObject* InSourceObject)
{
	ChoiceData = InChoiceData;
	SourceObject = InSourceObject;
	RefreshChoice();
}

void UMVInteractionChoicePopup::RefreshChoice()
{
	if (!ChoiceBox)
	{
		return;
	}

	if (PromptText)
	{
		PromptText->SetText(ChoiceData.PromptText);
		PromptText->SetVisibility(ChoiceData.PromptText.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible);
	}

	ChoiceBox->ClearChildren();

	int32 ChoiceIndex = 0;
	for (const FMVInteractionChoiceEntryData& Choice : ChoiceData.Choices)
	{
		const FName ButtonName = *FString::Printf(TEXT("InteractionChoiceEntry_%d"), ChoiceIndex++);
		UMVInteractionChoicePopupEntryButton* EntryButton =
			WidgetTree->ConstructWidget<UMVInteractionChoicePopupEntryButton>(
				UMVInteractionChoicePopupEntryButton::StaticClass(),
				ButtonName);
		if (!EntryButton)
		{
			continue;
		}

		FMVMenuEntryData EntryData = MakeEntryData(Choice);
		EntryButton->SetEntryData(EntryData);
		EntryButton->OnEntryButtonClicked.AddUniqueDynamic(
			this,
			&UMVInteractionChoicePopup::HandleChoiceButtonClicked);

		FButtonStyle ButtonStyle = EntryButton->GetStyle();
		ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(0.10f, 0.095f, 0.08f, 0.82f));
		ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.24f, 0.22f, 0.17f, 0.92f));
		ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.32f, 0.29f, 0.20f, 0.96f));
		ButtonStyle.Disabled.TintColor = FSlateColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.55f));
		EntryButton->SetStyle(ButtonStyle);

		UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			*FString::Printf(TEXT("%s_Text"), *ButtonName.ToString()));
		if (ButtonText)
		{
			ButtonText->SetText(ResolveEntryLabel(EntryData));
			ButtonText->SetJustification(ETextJustify::Center);
			ButtonText->SetAutoWrapText(true);
			ButtonText->SetColorAndOpacity(EntryData.bEnabled
				? FSlateColor(FLinearColor(0.88f, 0.84f, 0.75f, 1.0f))
				: FSlateColor(FLinearColor(0.48f, 0.46f, 0.42f, 1.0f)));
			EntryButton->SetContent(ButtonText);
		}

		if (UVerticalBoxSlot* EntrySlot = ChoiceBox->AddChildToVerticalBox(EntryButton))
		{
			EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}
	}
}

void UMVInteractionChoicePopup::NativeConstruct()
{
	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		BuildNativeChoiceTree();
	}

	Super::NativeConstruct();
	RefreshChoice();
}

void UMVInteractionChoicePopup::BuildNativeChoiceTree()
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

FMVMenuEntryData UMVInteractionChoicePopup::MakeEntryData(
	const FMVInteractionChoiceEntryData& Choice) const
{
	FMVMenuEntryData EntryData;
	EntryData.EntryId = Choice.ChoiceId;
	EntryData.Label = Choice.Label;
	EntryData.bEnabled = Choice.bEnabled;
	return EntryData;
}

FText UMVInteractionChoicePopup::ResolveEntryLabel(const FMVMenuEntryData& EntryData) const
{
	return EntryData.Label;
}

void UMVInteractionChoicePopup::HandleChoiceButtonClicked(UMVInteractionChoicePopupEntryButton* Button)
{
	if (!Button)
	{
		return;
	}

	const FMVMenuEntryData& EntryData = Button->GetEntryData();
	if (!EntryData.bEnabled)
	{
		return;
	}

	OnInteractionChoiceEntrySelected.Broadcast(SourceObject, EntryData);
	ClosePopup();
}
