#include "UI/Popup/MVMessagePopup.h"

#include "Components/TextBlock.h"

void UMVMessagePopup::SetMessageData(const FMVPopupMessageData& InMessageData)
{
	MessageData = InMessageData;
	SetAutoDismissSeconds(MessageData.Duration);
	SetMessageText(MessageData.MessageText);
}

void UMVMessagePopup::SetMessageText(FText InMessageText)
{
	MessageData.MessageText = InMessageText;

	if (MessageText)
	{
		MessageText->SetText(InMessageText);
	}
}
