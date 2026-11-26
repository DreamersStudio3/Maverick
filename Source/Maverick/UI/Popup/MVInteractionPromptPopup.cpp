#include "UI/Popup/MVInteractionPromptPopup.h"

#include "Components/TextBlock.h"
#include "UI/System/MVUIFont.h"

void UMVInteractionPromptPopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PromptText)
	{
		MVUIFont::AddDefaultFallback(*PromptText);
	}
	if (CandidateCounterText)
	{
		MVUIFont::AddDefaultFallback(*CandidateCounterText);
	}
	RefreshPromptWidgets();
}

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
	const FText CandidateHint = PromptData.CandidateCount > 1
		? FText::Format(
			NSLOCTEXT("MaverickInteraction", "InteractionCandidateHint", "{0} / {1}  ↑↓"),
			FText::AsNumber(PromptData.CandidateIndex + 1),
			FText::AsNumber(PromptData.CandidateCount))
		: FText::GetEmpty();
	if (PromptText)
	{
		PromptText->SetText(!CandidateCounterText && !CandidateHint.IsEmpty()
			? FText::Format(NSLOCTEXT("MaverickInteraction", "InteractionPromptWithCandidate", "{0}  {1}"),
				PromptData.PromptText, CandidateHint)
			: PromptData.PromptText);
	}

	const bool bHasInputKey = PromptData.InputKey.IsValid();
	const bool bShowEKeyIcon = InputKeyIcon && PromptData.InputKey == EKeys::E;
	if (InputKeyIcon)
	{
		InputKeyIcon->SetVisibility(bShowEKeyIcon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (InputKeyText)
	{
		InputKeyText->SetText(bHasInputKey
			? PromptData.InputKey.GetDisplayName(false)
			: FText::GetEmpty());
		if (InputKeyIcon)
		{
			InputKeyText->SetVisibility(bHasInputKey && !bShowEKeyIcon
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed);
		}
	}

	if (CandidateCounterText)
	{
		CandidateCounterText->SetText(CandidateHint);
		CandidateCounterText->SetVisibility(CandidateHint.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::HitTestInvisible);
	}
}
