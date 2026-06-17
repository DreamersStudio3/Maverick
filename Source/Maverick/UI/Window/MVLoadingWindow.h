#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWindowBase.h"
#include "MVLoadingWindow.generated.h"

UCLASS(Blueprintable)
class MAVERICK_API UMVLoadingWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVLoadingWindow(const FObjectInitializer& ObjectInitializer);
};
