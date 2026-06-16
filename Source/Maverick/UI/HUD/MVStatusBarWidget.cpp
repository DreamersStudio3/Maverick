#include "UI/HUD/MVStatusBarWidget.h"

#include "Components/ProgressBar.h"

void UMVStatusBarWidget::SetProgress(float CurrentValue, float MaxValue)
{
	TargetPercent = MaxValue > 0.0f ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;
}

void UMVStatusBarWidget::SetProgressImmediate(float CurrentValue, float MaxValue)
{
	TargetPercent = MaxValue > 0.0f ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;
	CurrentDisplayPercent = TargetPercent;

	if (Bar)
	{
		Bar->SetPercent(CurrentDisplayPercent);
	}
}

void UMVStatusBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Bar)
	{
		Bar->SetPercent(CurrentDisplayPercent);
	}
}

void UMVStatusBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Bar)
	{
		return;
	}

	CurrentDisplayPercent = FMath::FInterpTo(CurrentDisplayPercent, TargetPercent, InDeltaTime, InterpSpeed);
	if (FMath::IsNearlyEqual(CurrentDisplayPercent, TargetPercent, 0.001f))
	{
		CurrentDisplayPercent = TargetPercent;
	}

	Bar->SetPercent(CurrentDisplayPercent);
}
