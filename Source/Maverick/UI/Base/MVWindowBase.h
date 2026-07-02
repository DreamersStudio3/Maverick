#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVActivatableWidgetBase.h"
#include "MVWindowBase.generated.h"

class UMVWindowBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnWindowDeactivated, UMVWindowBase*, Window);

UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVWindowBase : public UMVActivatableWidgetBase
{
	GENERATED_BODY()

public:
	UMVWindowBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Window")
	FMVOnWindowDeactivated OnWindowDeactivated;

protected:
	virtual void NativeOnDeactivated() override;
};
