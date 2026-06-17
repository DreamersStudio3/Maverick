#include "UI/Base/MVWidgetBase.h"

UMVWidgetBase::UMVWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVWidgetBase::PlayFadeIn()
{
	FadeController.Play(*this, GetRenderOpacity(), 1.0f, FadeInSeconds, false);
}

void UMVWidgetBase::PlayFadeOut()
{
	FadeController.Play(*this, GetRenderOpacity(), 0.0f, FadeOutSeconds, true);
}

void UMVWidgetBase::RemoveFromParentWithFade()
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

void UMVWidgetBase::SetUIFadeDurations(float InFadeInSeconds, float InFadeOutSeconds)
{
	FadeInSeconds = FMath::Max(0.0f, InFadeInSeconds);
	FadeOutSeconds = FMath::Max(0.0f, InFadeOutSeconds);
}

void UMVWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	FadeController.Stop();
	if (bAutoFadeInOnConstruct)
	{
		FadeController.Play(*this, 0.0f, 1.0f, FadeInSeconds, false);
	}
}

void UMVWidgetBase::NativeDestruct()
{
	FadeController.Stop();

	Super::NativeDestruct();
}

void UMVWidgetBase::HandleFadeOutFinished()
{
	RemoveFromParent();
}
