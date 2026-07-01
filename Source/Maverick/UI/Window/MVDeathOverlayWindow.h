#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/Base/MVWindowBase.h"
#include "MVDeathOverlayWindow.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnDeathOverlayMinimumDisplayElapsed);

/**
 * 사망 연출 중 고정 사망 문구를 표시하는 오버레이 창.
 *
 * 이 창은 `YOU DIED` 같은 사망 알림을 fade in, hold, fade out으로 표시하고 표시 완료 시점을 알릴 뿐,
 * 로딩 창 전환이나 부활 처리는 직접 수행하지 않는다. DeathOverlay 이후 로딩 전환은 `UMVDeathRespawnFlow`,
 * 필드 초기화와 체크포인트 이동은 `UMVFieldTransitionSubsystem`이 담당한다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVDeathOverlayWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVDeathOverlayWindow(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Death")
	void SetDisplaySeconds(float InDisplaySeconds);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Death")
	bool HasMinimumDisplayElapsed() const { return bMinimumDisplayElapsed; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Death")
	FMVOnDeathOverlayMinimumDisplayElapsed OnMinimumDisplayElapsed;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void HandleFadeInFinished() override;
	virtual void HandleFadeOutFinished() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Death")
	TObjectPtr<UTextBlock> DeathText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Death", meta = (ClampMin = "0.0"))
	float MinimumDisplaySeconds = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Death|Animation", meta = (ClampMin = "0.0"))
	float DeathOverlayFadeInSeconds = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Death|Animation", meta = (ClampMin = "0.0"))
	float DeathOverlayFadeOutSeconds = 1.0f;

private:
	void BuildNativeWidgetTree();
	void RefreshDeathText();
	void ApplyDeathOverlayFadeDurations();
	void StartMinimumDisplayTimer();
	void ClearMinimumDisplayTimer();
	void BeginFadeOutAfterDisplay();
	void HandleMinimumDisplayTimeElapsed();

	FTimerHandle MinimumDisplayTimerHandle;
	bool bMinimumDisplayElapsed = false;
	bool bFadeInFinished = false;
	bool bFadeOutRequested = false;
};
