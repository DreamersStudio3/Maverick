#pragma once

#include "CoreMinimal.h"
#include "UI/World/MVWorldWidgetBase.h"
#include "MVTargetWidget.generated.h"

UCLASS(Blueprintable)
class MAVERICK_API UMVTargetWidget : public UMVWorldWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Target")
	void SetTargeted(bool bInTargeted);
};
