#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"

class UWidget;
class UWorld;

class MAVERICK_API FMVUIFadeController
{
public:
	using FOnFadeFinished = TFunction<void()>;

	FMVUIFadeController() = default;
	~FMVUIFadeController();

	FMVUIFadeController(const FMVUIFadeController&) = delete;
	FMVUIFadeController& operator=(const FMVUIFadeController&) = delete;

	void Play(UWidget& OwnerWidget, float FromOpacity, float ToOpacity, float Duration, bool bInFadingOut, FOnFadeFinished InOnFadeFinished = nullptr);
	void Stop();

	bool IsFadingOut() const { return bFadingOut; }
	bool IsPlaying() const { return bPlaying; }

private:
	void UpdateFade();
	void FinishFade();

	TWeakObjectPtr<UWidget> OwnerWidget;
	TWeakObjectPtr<UWorld> TimerWorld;
	FTimerHandle FadeTimerHandle;
	FOnFadeFinished OnFadeFinished;
	float FadeStartTimeSeconds = 0.0f;
	float ActiveFadeDuration = 0.0f;
	float ActiveFadeFromOpacity = 1.0f;
	float ActiveFadeToOpacity = 1.0f;
	bool bPlaying = false;
	bool bFadingOut = false;
};
