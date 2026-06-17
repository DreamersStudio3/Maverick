#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/Base/MVActivatableWidgetBase.h"
#include "UI/Base/MVUIFadeController.h"
#include "MVDialogueWindow.generated.h"

class UMVDialogueWindow;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDialogueWindowClosing, UMVDialogueWindow*, DialogueWindow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDialogueWindowClosed, UMVDialogueWindow*, DialogueWindow);

UCLASS(Blueprintable)
class MAVERICK_API UMVDialogueWindow : public UMVActivatableWidgetBase
{
	GENERATED_BODY()

public:
	UMVDialogueWindow(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void SetDialogueText(FText InDialogueText);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void SetAutoDismissSeconds(float InAutoDismissSeconds);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void SetMinimumSkipDelay(float InMinimumSkipDelay);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Dialogue")
	void CloseDialogue();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Dialogue")
	bool CanSkipDialogue() const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Dialogue")
	FMVOnDialogueWindowClosed OnDialogueWindowClosed;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Dialogue")
	FMVOnDialogueWindowClosing OnDialogueWindowClosing;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void HandleFadeInFinished() override;
	virtual bool NativeOnHandleBackAction() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Dialogue")
	TObjectPtr<UTextBlock> DialogueText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float AutoDismissSeconds = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float MinimumSkipDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float DialogueWindowFadeSeconds = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float DialogueTextFadeOutSeconds = 0.12f;

private:
	void HandleAutoDismissElapsed();
	void HandleMinimumSkipDelayElapsed();
	void HandleDialogueTextFadeOutFinished();
	void StartAutoDismissTimer();
	void StartMinimumSkipDelayTimer();
	void ClearDialogueTimers();
	void BeginDialogueWindowFadeOut();
	void BroadcastDialogueClosing();
	void BroadcastDialogueClosed();
	void RestoreGameInputMode() const;

	FMVUIFadeController DialogueTextFadeController;
	FTimerHandle AutoDismissTimerHandle;
	FTimerHandle MinimumSkipDelayTimerHandle;
	bool bClosedEventBroadcast = false;
	bool bClosingEventBroadcast = false;
	bool bCloseRequested = false;
	bool bFadeInFinished = false;
	bool bCanSkipDialogue = false;
};
