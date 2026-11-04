#include "UI/LevelUp/MVLevelUpStatNameWidget.h"

#include "Components/TextBlock.h"

UMVLevelUpStatNameWidget::UMVLevelUpStatNameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoFadeInOnConstruct = false;
}

void UMVLevelUpStatNameWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsDesignTime())
	{
		ApplyDisplay();
	}
}

void UMVLevelUpStatNameWidget::SetStatName(
	const FText& InDisplayName)
{
	CachedDisplayName = InDisplayName;
	bHasDisplayName = true;
	ApplyDisplay();
}

void UMVLevelUpStatNameWidget::ApplyDisplay()
{
	if (StatName && bHasDisplayName)
	{
		StatName->SetText(CachedDisplayName);
	}
}