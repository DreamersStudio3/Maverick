#include "UI/System/MVGameplayInputWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Input/CommonUIInputTypes.h"

UMVGameplayInputWidget::UMVGameplayInputWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;
	bIsBackHandler = false;
	bSupportsActivationFocus = true;
	bIsModal = true;
	bAutoRestoreFocus = false;

	SetIsFocusable(false);

	bSetVisibilityOnActivated = true;
	ActivatedVisibility = ESlateVisibility::SelfHitTestInvisible;

	bSetVisibilityOnDeactivated = true;
	DeactivatedVisibility = ESlateVisibility::Collapsed;
}

void UMVGameplayInputWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			TEXT("GameplayInputRoot"));
	}
}

TOptional<FUIInputConfig> UMVGameplayInputWidget::GetDesiredInputConfig() const
{
	FUIInputConfig InputConfig(
		ECommonInputMode::Game,
		EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown,
		EMouseLockMode::LockOnCapture,
		true);

	InputConfig.bIgnoreMoveInput = false;
	InputConfig.bIgnoreLookInput = false;

	return InputConfig;
}