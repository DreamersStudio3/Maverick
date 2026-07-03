#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVUIFadeController.h"
#include "MVDialoguePopup.generated.h"

class UTextBlock;
class UMVDialoguePopup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDialoguePopupClosing, UMVDialoguePopup*, DialoguePopup);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDialoguePopupClosed, UMVDialoguePopup*, DialoguePopup);

/**
 * 월드 조작을 장악하지 않고 화면 하단에 표시되는 대화 popup.
 *
 * 대화 text, skip delay, auto dismiss, 카메라 줌 복원 타이밍만 관리한다. Window stack을 쓰지 않으므로
 * 이동/카메라 입력은 CommonUI modal input config에 의해 차단되지 않는다.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVDialoguePopup : public UMVPopupBase
{
	GENERATED_BODY()

public:
	UMVDialoguePopup(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void SetDialogueText(FText InDialogueText);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void SetDialogueAutoDismissSeconds(float InAutoDismissSeconds);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void SetMinimumSkipDelay(float InMinimumSkipDelay);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void CloseDialogue();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Dialogue")
	bool CanSkipDialogue() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Dialogue")
	float GetDialogueFadeSeconds() const { return DialoguePopupFadeSeconds; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Dialogue")
	FMVOnDialoguePopupClosed OnDialoguePopupClosed;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Dialogue")
	FMVOnDialoguePopupClosing OnDialoguePopupClosing;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Dialogue")
	TObjectPtr<UTextBlock> DialogueText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float MinimumSkipDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float DialoguePopupFadeSeconds = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float DialogueTextFadeOutSeconds = 0.12f;

private:
	void BuildNativeDialogueTree();
	void HandleAutoDismissElapsed();
	void HandleMinimumSkipDelayElapsed();
	void HandleDialogueTextFadeOutFinished();
	void StartAutoDismissTimer();
	void StartMinimumSkipDelayTimer();
	void ClearDialogueTimers();
	void BeginDialoguePopupFadeOut();
	void BroadcastDialogueClosing();
	void BroadcastDialogueClosed();

	FMVUIFadeController DialogueTextFadeController;
	FTimerHandle AutoDismissTimerHandle;
	FTimerHandle MinimumSkipDelayTimerHandle;
	bool bClosedEventBroadcast = false;
	bool bClosingEventBroadcast = false;
	bool bCloseRequested = false;
	bool bConstructed = false;
	bool bCanSkipDialogue = false;
};
