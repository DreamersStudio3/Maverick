#pragma once

#include "CoreMinimal.h"
#include "MVItemAcquisitionNotifications.generated.h"

class AController;
class APawn;
class APlayerController;
class AMVPlayerCharacter;
class UMVInventoryComponent;
class UMVItemAcquisitionPanel;
class UMVUILayerBase;
class UMVUISubsystem;
class UMVWorldStateSubsystem;
struct FMVItemAcquisition;
struct FMVItemAcquisitionQueues;

/** 현재 로컬 플레이어의 습득을 구독하고 UI 계층 교체 중에도 대기열을 보관한다. */
UCLASS()
class UMVItemAcquisitionNotifications : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UMVUISubsystem& InUI);
	void Shutdown();
	void BindPlayer(AMVPlayerCharacter& Player, APlayerController& Controller);
	void AttachLayer(UMVUILayerBase& Layer);
	void DetachLayer();

private:
	void UnbindInventory();
	void HandleAcquired(const FMVItemAcquisition& Acquisition);
	void ResetNotifications();

	UFUNCTION()
	void HandlePawnControllerChanged(APawn* Pawn, AController* Controller);

	UFUNCTION()
	void HandleSlotLoaded(FString SlotName);

	UFUNCTION()
	void HandleSlotSaved(FString SlotName);

	UPROPERTY(Transient)
	TObjectPtr<UMVUISubsystem> UI;

	UPROPERTY(Transient)
	TObjectPtr<UMVItemAcquisitionPanel> Panel;

	TWeakObjectPtr<UMVWorldStateSubsystem> WorldState;
	TWeakObjectPtr<UMVInventoryComponent> Inventory;
	TWeakObjectPtr<APlayerController> LocalController;
	TWeakObjectPtr<UMVUILayerBase> CurrentLayer;
	FDelegateHandle AcquisitionHandle;
	TSharedPtr<FMVItemAcquisitionQueues> Queues;
	FString SaveSlotName;
	int32 SaveUserIndex = 0;
};
