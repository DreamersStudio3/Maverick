#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVStatusBarWidget.generated.h"

class UProgressBar;

UCLASS(Blueprintable)
class MAVERICK_API UMVStatusBarWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetProgress(float CurrentValue, float MaxValue);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetProgressImmediate(float CurrentValue, float MaxValue);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UProgressBar> Bar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status", meta = (ClampMin = "0.0"))
	float InterpSpeed = 5.0f;

private:
	float TargetPercent = 1.0f;
	float CurrentDisplayPercent = 1.0f;
};
