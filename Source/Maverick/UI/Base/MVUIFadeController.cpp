#include "UI/Base/MVUIFadeController.h"

#include "Components/Widget.h"
#include "Engine/World.h"

FMVUIFadeController::~FMVUIFadeController()
{
	Stop();
}

void FMVUIFadeController::Play(
	UWidget& InOwnerWidget,
	float FromOpacity,
	float ToOpacity,
	float Duration,
	bool bInFadingOut,
	FOnFadeFinished InOnFadeFinished)
{
	Stop();

	OwnerWidget = &InOwnerWidget;
	OnFadeFinished = MoveTemp(InOnFadeFinished);
	ActiveFadeFromOpacity = FromOpacity;
	ActiveFadeToOpacity = ToOpacity;
	ActiveFadeDuration = FMath::Max(0.0f, Duration);
	bPlaying = true;
	bFadingOut = bInFadingOut;

	InOwnerWidget.SetRenderOpacity(FromOpacity);

	UWorld* World = InOwnerWidget.GetWorld();
	if (!World || ActiveFadeDuration <= 0.0f)
	{
		InOwnerWidget.SetRenderOpacity(ToOpacity);
		FinishFade();
		return;
	}

	TimerWorld = World;
	FadeStartTimeSeconds = World->GetTimeSeconds();

	FTimerDelegate FadeDelegate = FTimerDelegate::CreateRaw(this, &FMVUIFadeController::UpdateFade);
	World->GetTimerManager().SetTimer(FadeTimerHandle, FadeDelegate, 0.01f, true);
}

void FMVUIFadeController::Stop()
{
	if (UWorld* World = TimerWorld.Get())
	{
		World->GetTimerManager().ClearTimer(FadeTimerHandle);
	}

	OwnerWidget.Reset();
	TimerWorld.Reset();
	OnFadeFinished.Reset();
	FadeStartTimeSeconds = 0.0f;
	ActiveFadeDuration = 0.0f;
	ActiveFadeFromOpacity = 1.0f;
	ActiveFadeToOpacity = 1.0f;
	bPlaying = false;
	bFadingOut = false;
}

void FMVUIFadeController::UpdateFade()
{
	UWidget* Widget = OwnerWidget.Get();
	if (!Widget)
	{
		Stop();
		return;
	}

	UWorld* World = TimerWorld.Get();
	if (!World || ActiveFadeDuration <= 0.0f)
	{
		Widget->SetRenderOpacity(ActiveFadeToOpacity);
		FinishFade();
		return;
	}

	const float Alpha = FMath::Clamp((World->GetTimeSeconds() - FadeStartTimeSeconds) / ActiveFadeDuration, 0.0f, 1.0f);
	Widget->SetRenderOpacity(FMath::Lerp(ActiveFadeFromOpacity, ActiveFadeToOpacity, Alpha));

	if (Alpha >= 1.0f)
	{
		Widget->SetRenderOpacity(ActiveFadeToOpacity);
		FinishFade();
	}
}

void FMVUIFadeController::FinishFade()
{
	if (UWorld* World = TimerWorld.Get())
	{
		World->GetTimerManager().ClearTimer(FadeTimerHandle);
	}

	FOnFadeFinished FinishedCallback = MoveTemp(OnFadeFinished);
	OwnerWidget.Reset();
	TimerWorld.Reset();
	OnFadeFinished.Reset();
	FadeStartTimeSeconds = 0.0f;
	ActiveFadeDuration = 0.0f;
	ActiveFadeFromOpacity = 1.0f;
	ActiveFadeToOpacity = 1.0f;
	bPlaying = false;
	bFadingOut = false;

	if (FinishedCallback)
	{
		FinishedCallback();
	}
}
