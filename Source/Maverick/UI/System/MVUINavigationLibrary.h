#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MVUINavigationLibrary.generated.h"

UCLASS()
class MAVERICK_API UMVUINavigationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Navigation")
	static void SetExplicitNavigation(UWidget* FromWidget, EUINavigation Direction, UWidget* ToWidget);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Navigation")
	static void SetNavigationStop(UWidget* FromWidget, EUINavigation Direction);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Navigation")
	static void SetCompassNavigation(
		UWidget* FromWidget,
		UWidget* UpWidget,
		UWidget* DownWidget,
		UWidget* LeftWidget,
		UWidget* RightWidget);
};
