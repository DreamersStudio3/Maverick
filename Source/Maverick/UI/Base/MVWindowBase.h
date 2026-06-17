#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVActivatableWidgetBase.h"
#include "MVWindowBase.generated.h"

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVWindowBase : public UMVActivatableWidgetBase
{
	GENERATED_BODY()

public:
	UMVWindowBase(const FObjectInitializer& ObjectInitializer);
};
