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

void UMVActivatableWidgetBase::PlayFadeIn()
{
	FadeController.Play(*this, GetRenderOpacity(), 1.0f, FadeInSeconds, false);
}

void UMVActivatableWidgetBase::PlayFadeOut()
{
	FadeController.Play(*this, GetRenderOpacity(), 0.0f, FadeOutSeconds, true);
}

void UMVActivatableWidgetBase::DeactivateWidgetWithFade()
{
	if (FadeController.IsFadingOut())
	{
		return;
	}

	FadeController.Play(*this, GetRenderOpacity(), 0.0f, FadeOutSeconds, true, [this]()
	{
		HandleFadeOutFinished();
	});
}

void UMVActivatableWidgetBase::SetUIFadeDurations(float InFadeInSeconds, float InFadeOutSeconds)
{
	FadeInSeconds = FMath::Max(0.0f, InFadeInSeconds);
	FadeOutSeconds = FMath::Max(0.0f, InFadeOutSeconds);
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

void UMVActivatableWidgetBase::NativeOnActivated()
{
	Super::NativeOnActivated();

	FadeController.Stop();
	if (bAutoFadeInOnActivated)
	{
		FadeController.Play(*this, 0.0f, 1.0f, FadeInSeconds, false);
	}
}

void UMVActivatableWidgetBase::NativeOnDeactivated()
{
	FadeController.Stop();

	Super::NativeOnDeactivated();
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

	DeactivateWidgetWithFade();
	return true;
}

void UMVActivatableWidgetBase::HandleFadeOutFinished()
{
	DeactivateWidget();
}
