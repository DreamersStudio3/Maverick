#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVUISubsystem.generated.h"

class UCommonActivatableWidget;
class UCameraComponent;
class AMVCharacterBase;
class UMVDialogueWindow;
class UMVHUDWidgetBase;
class UMVInteractionPromptPopup;
class UMVMessagePopup;
class UMVPIEActionTestWidget;
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
	UCommonActivatableWidget* ShowLoadingWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UCommonActivatableWidget* ShowDeathOverlay();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVInteractionPromptPopup* ShowInteractionPrompt(const FMVInteractionPromptData& PromptData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVInteractionPromptPopup* ShowInteractionPromptText(FText PromptText);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVDialogueWindow* ShowDialogueWindowText(FText DialogueText, float Duration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVDialogueWindow* ShowDialogueWindowTextWithTiming(FText DialogueText, float Duration = -1.0f, float MinimumSkipDelay = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Debug", meta = (AdvancedDisplay = "TargetCharacter"))
	UMVPIEActionTestWidget* ShowPIEActionTestPanel(AMVCharacterBase* TargetCharacter = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Debug")
	void HidePIEActionTestPanel();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Debug")
	bool IsPIEActionTestPanelActiveOrPending() const;

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
	UMVDialogueWindow* ShowDialogueWindowById(FName DialogueId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVMessagePopup* ShowPopupMessage(const FMVPopupMessageData& MessageData);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVMessagePopup* ShowPopupMessageText(FText MessageText, float Duration = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	UMVMessagePopup* ShowPopupMessageById(FName MessageId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI")
	void ClearAllUI();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI")
	UMVHUDWidgetBase* GetMainHUD() const { return CachedHUD; }

private:
	void HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS);

	UFUNCTION()
	void HandlePopupClosed(UMVPopupBase* ClosedPopup);

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
	UMVPIEActionTestWidget* OpenPIEActionTestPanel(AMVCharacterBase* TargetCharacter);
	AMVCharacterBase* ResolvePIEActionTestTargetCharacter(AMVCharacterBase* TargetCharacter) const;
	APlayerController* ResolvePIEActionTestPlayerController(const AMVCharacterBase* TargetCharacter) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVUILayerBase>> LayerStack;

	UPROPERTY(Transient)
	TObjectPtr<UMVHUDWidgetBase> CachedHUD;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionPromptPopup> ActiveInteractionPrompt;

	UPROPERTY(Transient)
	TObjectPtr<UMVDialogueWindow> ActiveDialogueWindow;

	UPROPERTY(Transient)
	TObjectPtr<UMVPopupBase> ActivePopup;

	UPROPERTY(Transient)
	TObjectPtr<UMVPIEActionTestWidget> ActivePIEActionTestWidget;

	TWeakObjectPtr<AMVCharacterBase> PendingPIEActionTestTargetCharacter;
	bool bHasPendingPIEActionTestPanel = false;

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
