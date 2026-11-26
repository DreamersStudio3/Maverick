#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVInteractionPromptPopup.generated.h"

class UTextBlock;

/** 현재 상호작용 대상의 안내 문구, 입력 키, 후보 순번을 표시한다. */
UCLASS(Blueprintable)
class MAVERICK_API UMVInteractionPromptPopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	void SetPromptText(FText InPromptText);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	void SetPromptData(const FMVInteractionPromptData& InPromptData);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Interaction")
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Interaction")
	TObjectPtr<UWidget> InputKeyIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Interaction")
	TObjectPtr<UTextBlock> InputKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Interaction")
	TObjectPtr<UTextBlock> CandidateCounterText;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|Interaction")
	FMVInteractionPromptData PromptData;

private:
	void RefreshPromptWidgets();
};
