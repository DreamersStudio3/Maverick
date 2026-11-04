#include "UI/LevelUp/MVLevelUpSummaryWidget.h"

#include "Components/TextBlock.h"

UMVLevelUpSummaryWidget::UMVLevelUpSummaryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoFadeInOnConstruct = false;
}

void UMVLevelUpSummaryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsDesignTime())
	{
		ApplyDisplay();
	}
}

void UMVLevelUpSummaryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyDisplay();
}

void UMVLevelUpSummaryWidget::SetSummary(const FMVProgressionEvaluation& Evaluation)
{
	CachedEvaluation = Evaluation;
	bHasSummary = true;

	ApplyDisplay();
}

void UMVLevelUpSummaryWidget::ApplyDisplay()
{
	if (!bHasSummary)
	{
		return;
	}

	const bool bValid = CachedEvaluation.IsSuccess();
	const FText UnavailableText = FText::FromString(TEXT("-"));

	if (Level_Current)
	{
		Level_Current->SetText(
			bValid
				? FText::AsNumber(CachedEvaluation.CurrentLevel)
				: UnavailableText);
	}

	if (Level_New)
	{
		Level_New->SetText(
			bValid
				? FText::AsNumber(CachedEvaluation.PreviewLevel)
				: UnavailableText);
	}

	if (CurrencyCurrent)
	{
		CurrencyCurrent->SetText(
			bValid
				? FText::AsNumber(CachedEvaluation.CurrentCurrency)
				: UnavailableText);
	}

	if (CurrencyNew)
	{
		CurrencyNew->SetText(
			bValid
				? FText::AsNumber(CachedEvaluation.RemainingCurrency)
				: UnavailableText);
	}

	if (CurrencyRequired)
	{
		CurrencyRequired->SetText(
			bValid
				? FText::AsNumber(CachedEvaluation.TotalCost)
				: UnavailableText);
	}
}