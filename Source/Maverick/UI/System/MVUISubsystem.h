#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/System/MVUIDataTypes.h"
#include "MVUISubsystem.generated.h"

class UCommonActivatableWidget;
class UMVHUDWidgetBase;
class UMVInteractionPromptPopup;
class UMVMessagePopup;
class UMVPopupBase;
class UMVUILayerBase;
class UMVWindowBase;

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
	UCommonActivatableWidget* PushPopupByClass(TSubclassOf<UMVPopupBase> PopupClass);

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
	void BindToPlayerDeath(UWorld* World);

	UFUNCTION()
	void HandlePlayerDeath();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMVUILayerBase>> LayerStack;

	UPROPERTY(Transient)
	TObjectPtr<UMVHUDWidgetBase> CachedHUD;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionPromptPopup> ActiveInteractionPrompt;
};
