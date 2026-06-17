#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVMessagePopup.generated.h"

class UTextBlock;

UCLASS(Blueprintable)
class MAVERICK_API UMVMessagePopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Message")
	void SetMessageData(const FMVPopupMessageData& InMessageData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Message")
	void SetMessageText(FText InMessageText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Message")
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|Message")
	FMVPopupMessageData MessageData;
};
