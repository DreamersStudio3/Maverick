#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "MVEnemyRewardSubsystem.generated.h"

class UMVInventoryComponent;
class AController;
class APawn;
enum class EMVFieldTransitionPhase : uint8;
struct FDataTableRowHandle;
struct FMVEnemyDropTableRow;

/**
 * 적 사망 보상을 현재 슬롯에 접수하고, 이 월드의 로컬 플레이어 가방에 지급한다.
 * 소지 한도나 전환 때문에 남은 수량은 슬롯에 보관하고 가방이 준비되거나 변경될 때 재시도한다.
 */
UCLASS()
class MAVERICK_API UMVEnemyRewardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void SubmitReward(const FMVEnemyDropTableRow& Row, const FDataTableRowHandle& Source);

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	UMVInventoryComponent* ObserveLocalInventory();
	void SchedulePendingRewards();
	void DeliverPendingRewards();

	UFUNCTION()
	void HandlePawnControllerChanged(APawn* Pawn, AController* Controller);
	UFUNCTION()
	void HandleSlotLoaded(FString SlotName);
	UFUNCTION()
	void HandleTransitionPhaseChanged(EMVFieldTransitionPhase Phase);
	UFUNCTION()
	void HandleInventoryChanged(FGameplayTag ItemTag);

	TWeakObjectPtr<UMVInventoryComponent> ObservedInventory;
	FTimerHandle RetryTimer;
};
