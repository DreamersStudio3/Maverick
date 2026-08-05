#include "UI/Popup/MVInteractionPromptPopup.h"

#include "Components/TextBlock.h"

void UMVInteractionPromptPopup::SetPromptText(FText InPromptText)
{
	PromptData.PromptText = InPromptText;
	RefreshPromptWidgets();
}

void UMVInteractionPromptPopup::SetPromptData(const FMVInteractionPromptData& InPromptData)
{
	PromptData = InPromptData;
	RefreshPromptWidgets();
}

void UMVInteractionPromptPopup::RefreshPromptWidgets()
{
	if (PromptText)
	{
		PromptText->SetText(PromptData.PromptText);
	}

	if (InputKeyText)
	{
		InputKeyText->SetText(PromptData.InputKey.IsValid()
			? PromptData.InputKey.GetDisplayName(false)
			: FText::GetEmpty());
	}

	if (CandidateCounterText)
	{
		CandidateCounterText->SetText(PromptData.CandidateCount > 1
			? FText::Format(
				NSLOCTEXT("MaverickInteraction", "InteractionCandidateCounter", "{0} / {1}"),
				FText::AsNumber(PromptData.CandidateIndex + 1),
				FText::AsNumber(PromptData.CandidateCount))
			: FText::GetEmpty());
	}
}
