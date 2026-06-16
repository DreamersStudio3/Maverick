#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVUILayerBase.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;
class UOverlay;

UCLASS(Blueprintable)
class MAVERICK_API UMVUILayerBase : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	UCommonActivatableWidget* PushWindow(TSubclassOf<UCommonActivatableWidget> WindowClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	UCommonActivatableWidget* PushPopup(TSubclassOf<UCommonActivatableWidget> PopupClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	UUserWidget* SetHUDByClass(TSubclassOf<UUserWidget> HUDClass, APlayerController* OwningPlayer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	void SetHUDWidget(UUserWidget* InHUDWidget);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	void AddWidgetLayerWidget(UUserWidget* InWidget);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	void ClearLayer();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Layer")
	UUserWidget* GetHUDWidget() const { return ActiveHUDWidget; }

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Layer")
	TObjectPtr<UCommonActivatableWidgetStack> WindowStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Layer")
	TObjectPtr<UOverlay> HUDLayer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Layer")
	TObjectPtr<UCommonActivatableWidgetStack> PopupStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Layer")
	TObjectPtr<UOverlay> WidgetLayer;

private:
	void BuildNativeLayerTree();
	void AddFullScreenOverlayChild(UOverlay* RootOverlay, UWidget* ChildWidget) const;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveHUDWidget;
};
