#pragma once

#include "CoreMinimal.h"
#include "Interface/MVActionInputHandlerInterface.h"
#include "Struct/MVHealingPotionTypes.h"
#include "Tables/MVHealingPotionTableTypes.h"
#include "UI/HUD/MVQuickSlotWidget.h"
#include "UObject/Object.h"
#include "MVPlayerConsumable.generated.h"

class AMVPlayerCharacter;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnHealingPotionStateChanged, const FMVHealingPotionRuntimeState&, HealingPotionState);

/**
 * 플레이어 전용 회복약 슬롯 런타임 서브모듈.
 *
 * 인벤토리 없이 고정 회복약 한 슬롯만 관리한다. 회복약 설정은 DataTable row에서 읽고, UseConsumable 입력을
 * 구독해 사용 가능 여부를 판정한다. 성공 시 사용 액션과 StatComponent HP 회복을 처리하고, 카운트가 없으면
 * 실패 액션으로 피드백을 준다. 카운트 변경은 HUD 퀵슬롯이 구독할 수 있도록 이벤트로 알리며, 사망 부활/월드
 * 리셋 때 기본 카운트로 복구된다.
 *
 * 라이프사이클:
 *   1) PlayerCharacter BeginPlay -> Initialize로 플레이어 소유자와 InputManager 이벤트를 바인딩하고 기본 상태를 초기화한다.
 *   2) UseConsumable 입력 -> TryUseHealingPotion으로 사용 액션/회복/카운트 감소를 처리한다.
 *   3) PlayerCharacter EndPlay -> Deinitialize로 입력 이벤트를 해제한다.
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
	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue);

	UFUNCTION()
	void HandleActionStarted(FName ActionTableName, FName ActionRowName);

	UFUNCTION()
	void HandleActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);

	void BindOwnerEvents();
	void UnbindOwnerEvents();
	void InitializeHealingPotionState();
	void ResolveHealingPotionData();
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Consumable", meta = (AllowPrivateAccess = "true"))
	FMVHealingPotionRuntimeState HealingPotionState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Consumable|HealingPotion", meta = (AllowPrivateAccess = "true"))
	FMVHealingPotionTableRow HealingPotionData;

	UPROPERTY(Transient)
	bool bHealingPotionUseActionRunning = false;
};
