#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVPopupBase.h"
#include "MVBossClearPopup.generated.h"

UCLASS(Blueprintable)
class MAVERICK_API UMVBossClearPopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UMVBossClearPopup(const FObjectInitializer& ObjectInitializer);
};
