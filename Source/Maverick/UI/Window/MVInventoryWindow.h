#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVUIFadeController.h"
#include "UI/Base/MVWindowBase.h"
#include "MVInventoryWindow.generated.h"

class AMVPlayerCharacter;
class UButton;
class UMVHUDWidgetBase;
class UMVInputManagerComponent;
class UMVInventoryPage;
class UUserWidget;
class UWidgetSwitcher;
enum class EMVFieldTransitionPhase : uint8;
struct FMVResolvedHitData;

/**
 * 필드 메뉴와 인벤토리 보기를 한 CommonUI 창에서 전환한다.
 * 활성 기간에 조종 플레이어의 게임 입력을 차단하고 피격·사망·전환·조종 대상 변경을 관찰한다.
 * 페이지를 페이드로 교체하는 동안 창이 초점을 유지하고 완료 후 대상 페이지에 넘긴다.
 * 종료할 때 페이지 전환과 입력 차단을 해제하고 다음 활성화는 필드 메뉴에서 시작한다.
 */
UCLASS()
class MAVERICK_API UMVInventoryWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVInventoryWindow(const FObjectInitializer& ObjectInitializer);
	static bool CanOpenForPlayer(const APlayerController* Controller);
	bool HasPresentation() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;

private:
	void ReleaseSession();
	void ConfirmFocusedChoice();
	void TransitionToPage(int32 PageIndex);

	UFUNCTION()
	void CloseInventory();

	UFUNCTION()
	void HandlePlayerHit(const FMVResolvedHitData& Hit);

	UFUNCTION()
	void HandlePawnChanged(APawn* Previous, APawn* Current);

	UFUNCTION()
	void HandleTransitionChanged(EMVFieldTransitionPhase Phase);

	UFUNCTION()
	void ShowInventoryPage();

	UFUNCTION()
	void ReturnToMenu();

	UPROPERTY()
	TSubclassOf<UUserWidget> MenuClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> InventoryClass;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetSwitcher> Pages;

	UPROPERTY(Transient)
	TObjectPtr<UButton> InventoryButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> BackButton;

	UPROPERTY(Transient)
	TObjectPtr<UMVInventoryPage> InventoryPage;

	TWeakObjectPtr<AMVPlayerCharacter> ObservedPlayer;
	TWeakObjectPtr<UMVInputManagerComponent> BlockedInput;
	TWeakObjectPtr<UMVHUDWidgetBase> BlockedHUD;
	FMVUIFadeController PageFade;
	FUIActionBindingHandle ConfirmInputBinding;
	ESlateVisibility PreviousHUDVisibility = ESlateVisibility::SelfHitTestInvisible;
};
