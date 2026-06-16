#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVPopupBase.h"
#include "MVInteractionPromptPopup.generated.h"

class UTextBlock;

UCLASS(Blueprintable)
class MAVERICK_API UMVInteractionPromptPopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	void SetPromptText(FText InPromptText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Interaction")
	TObjectPtr<UTextBlock> PromptText;
};
