#pragma once

#include "CoreMinimal.h"
#include "Input/NavigationReply.h"
#include "UI/Base/MVWindowBase.h"
#include "MVRestPointWindow.generated.h"

class UButton;
class UTextBlock;
enum class EMVRestPointDetail : uint8;

/**
 * 거점 이름과 작업 목록을 표시하고 선택을 거점 세션으로 전달한다.
 * 상세 창에서 돌아올 때 직전 항목에 초점을 돌린다.
 * Blueprint가 루트를 제공하지 않으면 기본 화면을 구성한다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVRestPointWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVRestPointWindow(const FObjectInitializer& ObjectInitializer);
	void ShowRestStatus(const FText& Status);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	virtual bool NativeOnHandleBackAction() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
	virtual FNavigationReply NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event,
		const FNavigationReply& DefaultReply) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RestPointName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> LeaveButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> LevelUpButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> PassiveSkillsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RetrySaveButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MenuMessage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InputGuide;

private:
	void BuildNativeMenu();
	void RefreshInputHints();
	void UnregisterConfirmInput();
	void HandleConfirmInput();
	UButton* GetCurrentMenuButton() const;
	UButton* MoveMenuSelection(EUINavigation Direction);
	void SetMenuSelection(UButton* Button);
	void RefreshSaveStatus();
	bool IsRestPending() const;

	UFUNCTION()
	void HandleRestClicked();
	void OpenDetail(EMVRestPointDetail Detail, UButton* Button);

	UFUNCTION()
	void HandleLevelUpClicked();

	UFUNCTION()
	void HandlePassiveSkillsClicked();

	UFUNCTION()
	void HandleRetrySaveClicked();

	UFUNCTION()
	void HandleLeaveClicked();

	TWeakObjectPtr<UButton> SelectedButton;
	FUIActionBindingHandle ConfirmInputBinding;
	bool bUsesNativeMenu = false;
	bool bShowingSaveFailure = false;
};
