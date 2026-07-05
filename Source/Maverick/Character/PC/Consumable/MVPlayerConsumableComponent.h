#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Struct/MVHealingPotionTypes.h"
#include "Tables/MVHealingPotionTableTypes.h"
#include "UI/HUD/MVQuickSlotWidget.h"
#include "MVPlayerConsumableComponent.generated.h"

class AMVPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnHealingPotionStateChanged, const FMVHealingPotionRuntimeState&, HealingPotionState);

/**
 * 플레이어 전용 회복약 슬롯 런타임 컴포넌트.
 *
 * 인벤토리 없이 고정 회복약 한 슬롯만 관리한다. UseConsumable 입력을 구독해 회복약 사용 가능 여부를 판정하고,
 * 성공 시 StatComponent에 HP 회복을 요청한 뒤 보유 카운트를 줄인다. 카운트 변경은 HUD 퀵슬롯이 구독할 수
 * 있도록 이벤트로 알리며, 사망 부활/월드 리셋 때 기본 카운트로 복구된다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> 플레이어 소유자와 InputManager 이벤트를 바인딩하고 기본 회복약 상태를 초기화한다.
 *   2) UseConsumable 입력 -> TryUseHealingPotion으로 사용 액션/회복/카운트 감소를 처리한다.
 *   3) EndPlay -> 입력 이벤트를 해제한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVPlayerConsumableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVPlayerConsumableComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	void Initialize(AMVPlayerCharacter* InOwnerCharacter);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	bool TryUseHealingPotion();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	bool ApplyHealingPotionEffect();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	void RestoreHealingPotionCountForWorldReset();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	void SetHealingPotionCount(int32 NewCount);

	UFUNCTION(BlueprintPure, Category = "Maverick|Consumable")
	FMVHealingPotionRuntimeState GetHealingPotionState() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Consumable")
	FMVQuickSlotViewData BuildHealingPotionQuickSlotViewData() const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Consumable|Event")
	FMVOnHealingPotionStateChanged OnHealingPotionStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Consumable|HealingPotion")
	FMVHealingPotionTableRow HealingPotionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Consumable|HealingPotion")
	FText HealingPotionHotKeyText;

private:
	UFUNCTION()
	void HandleActionInputSubmitted(int32 ActionId, FVector2D ControllerSpaceInput, bool bHasMovementInput);

	UFUNCTION()
	void HandleHPChanged(float CurrentValue, float MaxValue);

	void BindOwnerEvents();
	void UnbindOwnerEvents();
	void InitializeHealingPotionState();
	bool CanUseHealingPotion() const;
	void BroadcastHealingPotionStateChanged();

	UPROPERTY(Transient)
	TObjectPtr<AMVPlayerCharacter> OwnerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Consumable", meta = (AllowPrivateAccess = "true"))
	FMVHealingPotionRuntimeState HealingPotionState;
};
