#include "UI/LevelUp/MVLevelUpStatComparisonWidget.h"

#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"
#include "UI/LevelUp/MVLevelUpStatNameWidget.h"

UMVLevelUpStatComparisonWidget::UMVLevelUpStatComparisonWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoFadeInOnConstruct = false;
}

void UMVLevelUpStatComparisonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsDesignTime())
	{
		ApplyDisplay();
	}
}

void UMVLevelUpStatComparisonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyDisplay();
}

bool UMVLevelUpStatComparisonWidget::SetComparison(
	const FGameplayTag InStatId,
	const FText& InDisplayName,
	const float InCurrentValue,
	const float InPreviewValue,
	const bool bInLowerIsBetter)
{
	if (!InStatId.IsValid()
		|| !FMath::IsFinite(InCurrentValue)
		|| !FMath::IsFinite(InPreviewValue))
	{
		return false;
	}

	StatId = InStatId;
	CachedDisplayName = InDisplayName;

	CurrentValue = InCurrentValue;
	PreviewValue = InPreviewValue;

	bLowerIsBetter = bInLowerIsBetter;
	bHasComparison = true;

	ApplyDisplay();

	return true;
}

void UMVLevelUpStatComparisonWidget::ApplyDisplay()
{
	if (!bHasComparison)
	{
		return;
	}

	if (StatNameWidget)
	{
		StatNameWidget->SetStatName(CachedDisplayName);
	}

	FNumberFormattingOptions NumberFormatting;
	NumberFormatting.MinimumFractionalDigits = 0;
	NumberFormatting.MaximumFractionalDigits = 2;

	if (StatValueOld)
	{
		StatValueOld->SetText(FText::AsNumber(CurrentValue, &NumberFormatting));

		StatValueOld->SetColorAndOpacity(FSlateColor(UnchangedValueColor));
	}

	if (StatValueNew)
	{
		StatValueNew->SetText(FText::AsNumber(PreviewValue, &NumberFormatting));

		FLinearColor PreviewColor = UnchangedValueColor;

		if (!FMath::IsNearlyEqual(CurrentValue, PreviewValue))
		{
			const bool bImproved = bLowerIsBetter
				? PreviewValue < CurrentValue
				: PreviewValue > CurrentValue;

			PreviewColor = bImproved
				? ImprovedValueColor
				: ReducedValueColor;
		}

		StatValueNew->SetColorAndOpacity(FSlateColor(PreviewColor));
	}
}