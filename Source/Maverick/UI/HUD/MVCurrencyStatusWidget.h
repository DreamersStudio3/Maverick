#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVCurrencyStatusWidget.generated.h"

class UTextBlock;

UCLASS(Blueprintable)
class MAVERICK_API UMVCurrencyStatusWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Currency")
	void SetCurrency(int32 NewCurrency);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Currency")
	TObjectPtr<UTextBlock> CurrencyText;
};
