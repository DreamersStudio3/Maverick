#include "UI/HUD/MVCurrencyStatusWidget.h"

#include "Components/TextBlock.h"

UMVCurrencyStatusWidget::UMVCurrencyStatusWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoFadeInOnConstruct = false;
}

void UMVCurrencyStatusWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsDesignTime())
	{
		ApplyCurrencyDisplay();
	}
}

void UMVCurrencyStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyCurrencyDisplay();
}

void UMVCurrencyStatusWidget::SetCurrency(const int64 NewCurrency)
{
	CachedCurrency = NewCurrency;
	ApplyCurrencyDisplay();
}

void UMVCurrencyStatusWidget::ApplyCurrencyDisplay()
{
	if (CurrencyText)
	{
		CurrencyText->SetText(FText::AsNumber(CachedCurrency));
	}
}