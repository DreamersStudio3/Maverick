#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/Base/MVActivatableWidgetBase.h"
#include "MVDialogueWindow.generated.h"

class UMVDialogueWindow;
class UTextBlock;

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
	void CloseDialogue();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|Dialogue")
	FMVOnDialogueWindowClosed OnDialogueWindowClosed;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|Dialogue")
	TObjectPtr<UTextBlock> DialogueText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|Dialogue", meta = (ClampMin = "0.0"))
	float AutoDismissSeconds = 0.0f;

private:
	void HandleAutoDismissElapsed();
	void StartAutoDismissTimer();
	void BroadcastDialogueClosed();
	void RestoreGameInputMode() const;

	FTimerHandle AutoDismissTimerHandle;
	bool bClosedEventBroadcast = false;
};
