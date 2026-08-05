#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVHUDWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVHUDWidgetBase : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	virtual void RefreshHUD();
};
