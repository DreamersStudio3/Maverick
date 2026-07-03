#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Interaction/MVInteractionTypes.h"
#include "TimerManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVUISubsystem.generated.h"

class UCommonActivatableWidget;
class UCameraComponent;
class UMVDialogueWindow;
class UMVHUDWidgetBase;
class UMVInteractionChoicePopup;
class UMVInteractionMenuWindow;
class UMVInteractionPromptPopup;
class UMVLoadingWindow;
class UMVMessagePopup;
class UMVPopupBase;
class UMVUILayerBase;
class UMVWindowBase;
class USpringArmComponent;

UCLASS()
class MAVERICK_API UMVUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVUILayerBase* GetOrCreateRootLayer(UWorld* InWorld = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVUILayerBase* PushLayer(UWorld* InWorld = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void PopLayer();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UCommonActivatableWidget* PushWindowByClass(TSubclassOf<UMVWindowBase> WindowClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVPopupBase* PushPopupByClass(TSubclassOf<UMVPopupBase> PopupClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVHUDWidgetBase* ShowHUDByClass(TSubclassOf<UMVHUDWidgetBase> HUDClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVHUDWidgetBase* ShowDefaultHUD();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void HideHUD();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UCommonActivatableWidget* ShowLoadingWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UCommonActivatableWidget* ShowDeathOverlay();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVInteractionPromptPopup* ShowInteractionPrompt(const FMVInteractionPromptData& PromptData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVInteractionPromptPopup* ShowInteractionPromptText(FText PromptText);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	UMVInteractionMenuWindow* ShowInteractionMenu(const FMVInteractionMenuData& MenuData, UObject* SourceObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	void HideInteractionMenu();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Interaction")
	bool IsInteractionMenuActive() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	UMVInteractionChoicePopup* ShowInteractionChoice(const FMVInteractionChoiceData& ChoiceData, UObject* SourceObject = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	void HideInteractionChoice();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Interaction")
	bool IsInteractionChoiceActive() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	void BeginInteractionSession(UObject* SourceObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Interaction")
	void EndInteractionSession(UObject* SourceObject);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Interaction")
	bool IsInteractionSessionActive() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVDialogueWindow* ShowDialogueWindowText(FText DialogueText, float Duration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVDialogueWindow* ShowDialogueWindowTextWithTiming(FText DialogueText, float Duration = -1.0f, float MinimumSkipDelay = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Debug")
	UMVLoadingWindow* ShowLoadingWindowForTest(bool bUseNativeWindow = false);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Debug")
	void HideLoadingWindowForTest();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Debug")
	bool AdvanceLoadingGuideCardForTest();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void HideDialogueWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void SkipDialogueWindow();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI")
	bool CanSkipDialogueWindow() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|UI")
	bool CanUseInteractionPrompt() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void RestoreDialogueCameraZoom(float DurationOverride = -1.0f);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI")
	bool IsDialogueWindowActive() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|UI")
	bool IsDialogueWindowBlockingInteraction() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVDialogueWindow* ShowDialogueWindowByRow(FDataTableRowHandle DialogueRow);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVMessagePopup* ShowPopupMessage(const FMVPopupMessageData& MessageData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVMessagePopup* ShowPopupMessageText(FText MessageText, float Duration = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVMessagePopup* ShowPopupMessageById(FName MessageId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void ClearAllUI(bool bUseFadeOut = false);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void ResetToDefaultUI();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI")
	UMVHUDWidgetBase* GetMainHUD() const { return CachedHUD; }

private:
	void HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS);

	UFUNCTION()
	void HandlePopupClosed(UMVPopupBase* ClosedPopup);

	UFUNCTION()
	void HandleInteractionMenuClosed(UMVInteractionMenuWindow* ClosedMenuWindow);

	UFUNCTION()
	void HandleInteractionChoiceClosed(UMVPopupBase* ClosedChoicePopup);

	UFUNCTION()
	void HandleDialogueWindowClosed(UMVDialogueWindow* ClosedDialogueWindow);

	UFUNCTION()
	void HandleDialogueWindowClosing(UMVDialogueWindow* ClosingDialogueWindow);

	void HandleDialoguePromptRestoreDelayElapsed();
	void ApplyDialogueCameraZoom(float DurationOverride = -1.0f);
	void StartDialogueCameraZoom(bool bInRestoring, float DurationOverride = -1.0f);
	float ResolveDialogueCameraZoomDuration(float DurationOverride, bool bRestoring) const;
	void UpdateDialogueCameraZoom();
	void FinishDialogueCameraZoom();
	void ClearAllUIInternal(bool bUseFadeOut);
	void ResetUITrackingState();
	bool IsPopupActive(const UMVPopupBase* Popup) const;
	bool IsDialogueWindowActive(const UMVDialogueWindow* DialogueWindow) const;
	bool IsDialogueWindowPresent(const UMVDialogueWindow* DialogueWindow) const;
	void CloseActivePopupImmediately();
	void CloseActivePopup();
	UMVDialogueWindow* OpenDialogueWindowText(FText DialogueText, float Duration, float MinimumSkipDelay);
	void QueueDialogueWindowText(FText DialogueText, float Duration, float MinimumSkipDelay);
	void TryOpenPendingDialogueWindow();
	void TrackActivePopup(UMVPopupBase* Popup);
	void TrackActiveDialogueWindow(UMVDialogueWindow* DialogueWindow);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVUILayerBase>> LayerStack;

	UPROPERTY(Transient)
	TObjectPtr<UMVHUDWidgetBase> CachedHUD;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionPromptPopup> ActiveInteractionPrompt;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionMenuWindow> ActiveInteractionMenuWindow;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionChoicePopup> ActiveInteractionChoicePopup;

	UPROPERTY(Transient)
	TObjectPtr<UMVDialogueWindow> ActiveDialogueWindow;

	UPROPERTY(Transient)
	TObjectPtr<UMVPopupBase> ActivePopup;

	UPROPERTY(Transient)
	TObjectPtr<UMVLoadingWindow> ActiveLoadingWindowForTest;

	TWeakObjectPtr<UObject> ActiveInteractionMenuSource;
	TWeakObjectPtr<UObject> ActiveInteractionChoiceSource;
	TArray<TWeakObjectPtr<UObject>> InteractionSessionSources;

	TWeakObjectPtr<USpringArmComponent> DialogueZoomSpringArm;
	TWeakObjectPtr<UCameraComponent> DialogueZoomCamera;
	FTimerHandle DialoguePromptRestoreDelayTimerHandle;
	FTimerHandle DialogueCameraZoomTimerHandle;
	float DialogueZoomOriginalArmLength = 0.0f;
	float DialogueZoomStartArmLength = 0.0f;
	float DialogueZoomTargetArmLength = 0.0f;
	float DialogueZoomOriginalFOV = 0.0f;
	float DialogueZoomStartFOV = 0.0f;
	float DialogueZoomTargetFOV = 0.0f;
	float DialogueZoomStartTimeSeconds = 0.0f;
	float DialogueZoomDurationSeconds = 0.0f;
	float DialogueZoomDecelerationExponent = 2.0f;
	bool bDialoguePromptRestoreDelayActive = false;
	bool bDialogueCameraZoomApplied = false;
	bool bDialogueCameraZoomRestoring = false;

	FText PendingDialogueText;
	float PendingDialogueDuration = -1.0f;
	float PendingDialogueMinimumSkipDelay = -1.0f;
	bool bHasPendingDialogueRequest = false;
};
