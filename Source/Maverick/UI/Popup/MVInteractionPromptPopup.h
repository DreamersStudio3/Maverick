#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVInteractionPromptPopup.generated.h"

class UTextBlock;

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
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Interaction")
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Interaction")
	TObjectPtr<UTextBlock> InputKeyText;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|Interaction")
	FMVInteractionPromptData PromptData;

private:
	void RefreshPromptWidgets();
};
