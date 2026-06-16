#include "UI/Popup/MVDialoguePopup.h"

#include "Components/TextBlock.h"

void UMVDialoguePopup::SetDialogueText(FText InDialogueText)
{
	if (DialogueText)
	{
		DialogueText->SetText(InDialogueText);
	}
}
