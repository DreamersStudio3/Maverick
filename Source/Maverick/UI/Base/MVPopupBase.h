#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVPopupBase.generated.h"

class UMVPopupBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnPopupClosed, UMVPopupBase*, Popup);

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVPopupBase : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVPopupBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Popup")
	void ClosePopup();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Popup")
	void ClosePopupImmediately();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Popup")
	void SetAutoDismissSeconds(float InAutoDismissSeconds);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Popup")
	bool IsClosing() const { return IsFadingOut(); }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Popup")
	FMVOnPopupClosed OnPopupClosed;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void HandleFadeOutFinished() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Popup", meta = (ClampMin = "0.0"))
	float AutoDismissSeconds = 0.0f;

private:
	void HandleAutoDismissElapsed();
	void StartAutoDismissTimer();
	void BroadcastPopupClosed();

	FTimerHandle AutoDismissTimerHandle;
	bool bClosedEventBroadcast = false;
};
