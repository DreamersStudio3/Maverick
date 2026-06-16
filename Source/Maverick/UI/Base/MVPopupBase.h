#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/Base/MVActivatableWidgetBase.h"
#include "MVPopupBase.generated.h"

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVPopupBase : public UMVActivatableWidgetBase
{
	GENERATED_BODY()

public:
	UMVPopupBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Popup")
	void ClosePopup();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Popup")
	void SetAutoDismissSeconds(float InAutoDismissSeconds);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Popup", meta = (ClampMin = "0.0"))
	float AutoDismissSeconds = 0.0f;

private:
	void HandleAutoDismissElapsed();
	void StartAutoDismissTimer();

	FTimerHandle AutoDismissTimerHandle;
};
