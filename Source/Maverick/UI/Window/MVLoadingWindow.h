#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/Base/MVUIFadeController.h"
#include "MVLoadingWindow.generated.h"

class UProgressBar;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnLoadingWindowProgressChanged, float, Progress, FText, StepText);

USTRUCT(BlueprintType)
struct FMVLoadingGuideCard
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|Loading")
	FString GuideId;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|Loading")
	FText Title;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|Loading")
	FText Body;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|Loading")
	int32 Priority = 0;
};

/**
 * 로딩 중 초기화 진행률과 도움말 카드 UI를 표시하는 창.
 *
 * RespawnSubsystem이 넘기는 progress/step text를 표시하고,
 * GameGuide 테이블에서 로딩 노출이 허용된 항목을 카드/캐러셀로 렌더링한다.
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

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Loading")
	void LoadLoadingGuideCards();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Loading")
	bool AdvanceLoadingGuideCard();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Loading")
	FMVLoadingGuideCard GetCurrentLoadingGuideCard() const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Loading")
	FMVOnLoadingWindowProgressChanged OnLoadingProgressChanged;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Loading", meta = (ClampMin = "0.0"))
	float GuideCardTransitionFadeSeconds = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Loading")
	TSoftObjectPtr<UInputAction> LoadingGuideAdvanceInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Loading")
	FKey LoadingGuideAdvanceFallbackKey = EKeys::E;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Loading")
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Loading")
	TObjectPtr<UTextBlock> LoadingStepTextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Loading")
	TObjectPtr<UTextBlock> GuideTitleTextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Loading")
	TObjectPtr<UTextBlock> GuideBodyTextBlock;

private:
	void BuildNativeWidgetTree();
	void RefreshLoadingProgress();
	void RefreshLoadingGuideCard();
	bool TryAdvanceLoadingGuideCardFromInput(const FKey& Key);
	bool IsLoadingGuideAdvanceKey(const FKey& Key) const;
	bool IsMappedLoadingGuideAdvanceActionKey(const FKey& Key) const;
	void StartLoadingGuideCardTransition(int32 NewGuideCardIndex);
	void HandleLoadingGuideCardFadeOutFinished();
	void HandleLoadingGuideCardFadeInFinished();
	void StopLoadingGuideCardTransition();

	float LoadingProgress = 0.0f;
	FText LoadingStepText;
	TArray<FMVLoadingGuideCard> LoadingGuideCards;
	int32 LoadingGuideCardIndex = INDEX_NONE;
	int32 PendingLoadingGuideCardIndex = INDEX_NONE;
	bool bGuideCardTransitionInProgress = false;
	bool bAdvanceGuideCardAfterTransition = false;
	FMVUIFadeController GuideTitleFadeController;
	FMVUIFadeController GuideBodyFadeController;
};
