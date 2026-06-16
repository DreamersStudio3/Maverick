#include "UI/Base/MVActivatableWidgetBase.h"

UMVActivatableWidgetBase::UMVActivatableWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsBackHandler = true;
	bSupportsActivationFocus = true;
	bIsModal = true;
	bAutoRestoreFocus = true;

	bSetVisibilityOnActivated = true;
	ActivatedVisibility = ESlateVisibility::SelfHitTestInvisible;
	bSetVisibilityOnDeactivated = true;
	DeactivatedVisibility = ESlateVisibility::Collapsed;
}

TOptional<FUIInputConfig> UMVActivatableWidgetBase::GetDesiredInputConfig() const
{
	if (!bUseDesiredInputConfig)
	{
		return Super::GetDesiredInputConfig();
	}

	FUIInputConfig InputConfig(
		DesiredInputMode,
		DesiredMouseCaptureMode,
		DesiredMouseLockMode,
		bHideCursorDuringViewportCapture);
	InputConfig.bIgnoreMoveInput = bIgnoreMoveInput;
	InputConfig.bIgnoreLookInput = bIgnoreLookInput;
	return InputConfig;
}

UWidget* UMVActivatableWidgetBase::NativeGetDesiredFocusTarget() const
{
	if (InitialFocusTarget)
	{
		return InitialFocusTarget;
	}

	return Super::NativeGetDesiredFocusTarget();
}

bool UMVActivatableWidgetBase::NativeOnHandleBackAction()
{
	if (!bCloseOnBack)
	{
		return false;
	}

	return Super::NativeOnHandleBackAction();
}
