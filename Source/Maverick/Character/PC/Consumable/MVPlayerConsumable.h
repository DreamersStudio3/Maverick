#pragma once

#include "CoreMinimal.h"
#include "Interface/MVActionInputHandlerInterface.h"
#include "Struct/MVHealingPotionTypes.h"
#include "Tables/MVHealingPotionTableTypes.h"
#include "UI/HUD/MVQuickSlotWidget.h"
#include "UObject/Object.h"
#include "MVPlayerConsumable.generated.h"

class AMVPlayerCharacter;
class UMVInventoryComponent;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnHealingPotionStateChanged, const FMVHealingPotionRuntimeState&, HealingPotionState);

/**
 * 고정 회복약 슬롯의 입력, 사용 액션과 회복 효과를 담당한다.
 * 가방에서 수량을 읽고 회복 효과가 들어가는 시점에 차감한다. 휴식과 부활은 기본 수량만 복구한다.
 * 초기화와 종료 시 가방·입력·전투 이벤트를 연결하거나 해제하며, HUD에는 가방 수량을 전달한다.
 */
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class MAVERICK_API UMVPlayerConsumable : public UObject, public IMVActionInputHandlerInterface
{
	GENERATED_BODY()

public:
	UMVPlayerConsumable();

	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	void Initialize(AMVPlayerCharacter* InOwnerCharacter);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	void Deinitialize();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	bool TryUseHealingPotion();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	bool ApplyHealingPotionEffect();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	void RestoreConsumableCountsForWorldReset();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	void SetHealingPotionCount(int32 NewCount);

	UFUNCTION(BlueprintPure, Category = "Maverick|Consumable")
	FMVHealingPotionRuntimeState GetHealingPotionState() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	FMVQuickSlotViewData BuildHealingPotionQuickSlotViewData() const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Consumable|Event")
	FMVOnHealingPotionStateChanged OnHealingPotionStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Consumable|HealingPotion", meta = (RowType = "/Script/Maverick.MVHealingPotionTableRow"))
	FDataTableRowHandle HealingPotionDataRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Consumable|HealingPotion")
	FText HealingPotionHotKeyText;

	UFUNCTION(BlueprintPure, Category = "Maverick|Consumable")
	bool IsHealingPotionUseActionRunning() const;

	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;

private:
	friend class UMVInventoryComponent;

	UFUNCTION()
	void HandleInventoryChanged(FGameplayTag ItemTag);

	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleActionStarted(FName ActionTableName, FName ActionRowName);

	UFUNCTION()
	void HandleActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);

	void BindOwnerEvents();
	void UnbindOwnerEvents();
	FMVHealingPotionTableRow ResolveHealingPotionData() const;
	void GetDefaultHealingPotionStack(FGameplayTag& ItemTag, int32& Count) const;
	int32 GetHealingPotionCount() const;
	bool CanUseHealingPotion() const;
	bool HasHealingPotionUseAction() const;
	bool HasHealingPotionFailedAction() const;
	bool CanPlayHealingPotionFailedAction() const;
	bool CanTransitionActiveActionForHealingPotion() const;
	bool TryStartHealingPotionAction(const FDataTableRowHandle& ActionRow, FName StartSection);
	bool TryPlayHealingPotionFailedAction();
	bool IsHealingPotionUseAction(FName ActionTableName, FName ActionRowName) const;
	bool IsHealingPotionFailedAction(FName ActionTableName, FName ActionRowName) const;
	FName ResolveHealingPotionActionTableName(const FDataTableRowHandle& ActionRow) const;
	void RestoreHealingPotionCountToDefault();
	void BroadcastHealingPotionStateChanged();

	UPROPERTY(Transient)
	TObjectPtr<AMVPlayerCharacter> OwnerCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Consumable|HealingPotion", meta = (AllowPrivateAccess = "true"))
	FMVHealingPotionTableRow HealingPotionData;

	UPROPERTY(Transient)
	bool bHealingPotionUseActionRunning = false;

	bool bHealingPotionEffectApplied = false;
};
