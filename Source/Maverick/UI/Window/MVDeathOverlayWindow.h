#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/Base/MVWindowBase.h"
#include "MVDeathOverlayWindow.generated.h"

UCLASS(Blueprintable)
class MAVERICK_API UMVDeathOverlayWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVDeathOverlayWindow(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Death")
	void SetDisplaySeconds(float InDisplaySeconds);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Death", meta = (ClampMin = "0.0"))
	float DisplaySeconds = 3.0f;

private:
	void HandleDisplayTimeElapsed();

	FTimerHandle DisplayTimerHandle;
};
