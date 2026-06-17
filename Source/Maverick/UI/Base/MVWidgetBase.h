#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Base/MVUIFadeController.h"
#include "MVWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UMVWidgetBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void PlayFadeIn();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void PlayFadeOut();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void RemoveFromParentWithFade();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Animation")
	void SetUIFadeDurations(float InFadeInSeconds, float InFadeOutSeconds);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Animation")
	bool IsFadingOut() const { return FadeController.IsFadingOut(); }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void HandleFadeOutFinished();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Animation")
	bool bAutoFadeInOnConstruct = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Animation", meta = (ClampMin = "0.0"))
	float FadeInSeconds = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Animation", meta = (ClampMin = "0.0"))
	float FadeOutSeconds = 0.12f;

private:
	FMVUIFadeController FadeController;
};
