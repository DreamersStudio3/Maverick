#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVStatusBarWidget.generated.h"

class UProgressBar;
class USizeBox;
class UTextBlock;

UCLASS(Blueprintable)
class MAVERICK_API UMVStatusBarWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetLabel(FText InLabel);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetFillColor(FLinearColor InFillColor);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetBarSize(float InWidth, float InHeight);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetLabelVisible(bool bInVisible);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetValueVisible(bool bInVisible);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetRecentLossHoldLocked(bool bInLocked);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetProgress(float CurrentValue, float MaxValue);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Status")
	void SetProgressImmediate(float CurrentValue, float MaxValue);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UProgressBar> Bar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UProgressBar> RecentLossBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<UTextBlock> ValueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Status")
	TObjectPtr<USizeBox> BarSizeBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status")
	FText StatusLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status")
	FLinearColor FillColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status")
	FLinearColor RecentLossColor = FLinearColor(0.95f, 0.75f, 0.16f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status", meta = (ClampMin = "0.0"))
	float DesiredBarWidth = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status", meta = (ClampMin = "0.0"))
	float DesiredBarHeight = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status")
	bool bShowLabelText = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status")
	bool bShowValueText = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status", meta = (ClampMin = "0.0"))
	float RecentLossHoldTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|Status", meta = (ClampMin = "0.0"))
	float RecentLossMinAmount = 2.0f;

private:
	void BuildNativeWidgetTree();
	void UpdateBarStyle();
	void UpdateBarSize();
	void UpdateTextBlocks();
	void UpdateTextVisibility();

	float TargetPercent = 1.0f;
	float CurrentDisplayPercent = 1.0f;
	float RecentLossDisplayPercent = 1.0f;
	float RecentLossHoldRemaining = 0.0f;
	float LastCurrentValue = 1.0f;
	float LastMaxValue = 1.0f;
	bool bHasProgressValue = false;
};
