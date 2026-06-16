#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVUISubsystem.generated.h"

class UCommonActivatableWidget;
class UMVHUDWidgetBase;
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
};
