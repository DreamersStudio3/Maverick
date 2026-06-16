#include "UI/Popup/MVInteractionPromptPopup.h"

#include "Components/TextBlock.h"

void UMVInteractionPromptPopup::SetPromptText(FText InPromptText)
{
	if (PromptText)
	{
		PromptText->SetText(InPromptText);
	}
}
