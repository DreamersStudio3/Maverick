#include "UI/HUD/MVCurrencyStatusWidget.h"

#include "Components/TextBlock.h"

void UMVCurrencyStatusWidget::SetCurrency(int32 NewCurrency)
{
	if (CurrencyText)
	{
		CurrencyText->SetText(FText::AsNumber(NewCurrency));
	}
}
