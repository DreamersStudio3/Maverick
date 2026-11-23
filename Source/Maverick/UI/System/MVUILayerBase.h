#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "MVUILayerBase.generated.h"

class UCommonActivatableWidget;
class UMVPopupBase;
class UOverlay;

/** 전환 대상이 제거되면 아래쪽 창 또는 기본 게임 화면으로 돌아가는 기본 창 스택. */
UCLASS()
class UMVWindowStack : public UCommonActivatableWidgetStack
{
	GENERATED_BODY()

public:
	UMVWindowStack(const FObjectInitializer& ObjectInitializer);
};

/**
 * HUD·창 스택·알림의 표시 계층을 소유하고 창 생성 전에 필요한 문맥을 연결한다.
 * 스택에 창이 있거나 화면 전환 중이면 기본 입력 위젯의 메뉴 차단을 유지한다.
 * 마지막 창의 전환이 끝나면 차단을 해제하며 실제 입력 적용은 CommonUI가 담당한다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVUILayerBase : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	UCommonActivatableWidget* PushWindow(TSubclassOf<UCommonActivatableWidget> WindowClass);

	UCommonActivatableWidget* PushWindow(
		TSubclassOf<UCommonActivatableWidget> WindowClass,
		TFunctionRef<void(UCommonActivatableWidget&)> Initialize);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Layer")
	UCommonActivatableWidget* FindWindowByClass(TSubclassOf<UCommonActivatableWidget> WindowClass) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	bool RemoveWindow(UCommonActivatableWidget* Window);

	bool HasWindowInput() const;
	
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	UMVPopupBase* PushPopup(TSubclassOf<UMVPopupBase> PopupClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	UUserWidget* SetHUDByClass(TSubclassOf<UUserWidget> HUDClass, APlayerController* OwningPlayer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	void SetHUDWidget(UUserWidget* InHUDWidget);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Layer")
	void HideHUDWidgetWithFade();

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
	TObjectPtr<UOverlay> PopupLayer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Layer")
	TObjectPtr<UOverlay> WidgetLayer;

private:
	void BuildNativeLayerTree();
	void AddFullScreenOverlayChild(UOverlay* RootOverlay, UWidget* ChildWidget) const;
	void RefreshWindowInputBlock();
	void HandleWindowTransitionChanged(UCommonActivatableWidgetContainerBase* Container, bool bIsTransitioning);
	void HandleDisplayedWindowChanged(UCommonActivatableWidget* Window);
	bool bWindowStackTransitioning = false;

	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> GameplayInputWidget;
	
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ActiveHUDWidget;
};
