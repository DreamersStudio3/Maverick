#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWindowBase.h"
#include "MVLoadingWindow.generated.h"

class UProgressBar;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnLoadingWindowProgressChanged, float, Progress, FText, StepText);

/**
 * 로딩 중 초기화 진행률과 도움말 카드 UI를 표시하는 창.
 *
 * 현재는 RespawnSubsystem이 넘기는 progress/step text를 표시할 수 있는 기본 계약을 제공한다.
 * GameGuide 테이블에서 로딩 노출이 허용된 항목을 카드/캐러셀로 렌더링하는 표시 계층은 이후 확장한다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVLoadingWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVLoadingWindow(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Loading")
	void SetLoadingProgress(float InProgress, FText InStepText);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Loading")
	float GetLoadingProgress() const { return LoadingProgress; }

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Loading")
	FText GetLoadingStepText() const { return LoadingStepText; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Loading")
	FMVOnLoadingWindowProgressChanged OnLoadingProgressChanged;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Loading")
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Loading")
	TObjectPtr<UTextBlock> LoadingStepTextBlock;

private:
	void RefreshLoadingProgress();

	float LoadingProgress = 0.0f;
	FText LoadingStepText;
};
