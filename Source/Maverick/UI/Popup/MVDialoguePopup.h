#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVPopupBase.h"
#include "MVDialoguePopup.generated.h"

class UTextBlock;

UCLASS(Blueprintable)
class MAVERICK_API UMVDialoguePopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void SetDialogueText(FText InDialogueText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Dialogue")
	TObjectPtr<UTextBlock> DialogueText;
};
