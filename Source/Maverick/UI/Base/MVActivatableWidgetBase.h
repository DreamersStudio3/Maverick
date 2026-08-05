#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "UI/Base/MVUIFadeController.h"
#include "MVActivatableWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVActivatableWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UMVActivatableWidgetBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void PlayFadeIn();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void PlayFadeOut();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void DeactivateWidgetWithFade();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void SetUIFadeDurations(float InFadeInSeconds, float InFadeOutSeconds);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Animation")
	bool IsFadingOut() const { return FadeController.IsFadingOut(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Animation")
	bool IsFading() const { return FadeController.IsPlaying(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Animation")
	float GetFadeInSeconds() const { return FadeInSeconds; }

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Animation")
	float GetFadeOutSeconds() const { return FadeOutSeconds; }

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;
	virtual void HandleFadeInFinished();
	virtual void HandleFadeOutFinished();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input")
	bool bUseDesiredInputConfig = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	ECommonInputMode DesiredInputMode = ECommonInputMode::Menu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	EMouseCaptureMode DesiredMouseCaptureMode = EMouseCaptureMode::NoCapture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	EMouseLockMode DesiredMouseLockMode = EMouseLockMode::DoNotLock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	bool bHideCursorDuringViewportCapture = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	bool bIgnoreMoveInput = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input", meta = (EditCondition = "bUseDesiredInputConfig"))
	bool bIgnoreLookInput = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Input")
	bool bCloseOnBack = true;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Input")
	TObjectPtr<UWidget> InitialFocusTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Animation")
	bool bAutoFadeInOnActivated = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Animation", meta = (ClampMin = "0.0"))
	float FadeInSeconds = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Animation", meta = (ClampMin = "0.0"))
	float FadeOutSeconds = 0.12f;

private:
	FMVUIFadeController FadeController;
};
