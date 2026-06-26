#include "UI/Window/MVLoadingWindow.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UMVLoadingWindow::UMVLoadingWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCloseOnBack = false;
}

void UMVLoadingWindow::SetLoadingProgress(const float InProgress, FText InStepText)
{
	LoadingProgress = FMath::Clamp(InProgress, 0.0f, 1.0f);
	LoadingStepText = MoveTemp(InStepText);
	RefreshLoadingProgress();
	OnLoadingProgressChanged.Broadcast(LoadingProgress, LoadingStepText);
}

void UMVLoadingWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RefreshLoadingProgress();
}

void UMVLoadingWindow::RefreshLoadingProgress()
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(LoadingProgress);
	}

	if (LoadingStepTextBlock)
	{
		LoadingStepTextBlock->SetText(LoadingStepText);
	}
}
