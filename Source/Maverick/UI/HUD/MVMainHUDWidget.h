#pragma once

#include "CoreMinimal.h"
#include "Struct/MVHealingPotionTypes.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "MVMainHUDWidget.generated.h"

class UMVBossHPBarWidget;
class UMVCurrencyStatusWidget;
class UMVPlayerConsumableComponent;
class UMVPlayerStatusWidget;
class UMVQuickSlotWidget;
class UMVStatComponent;

UCLASS(Blueprintable)
class MAVERICK_API UMVMainHUDWidget : public UMVHUDWidgetBase
{
	GENERATED_BODY()

public:
	virtual void RefreshHUD() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void InitBossStatus(FText BossName, float MaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void BindBossStatus(UMVStatComponent* BossStatComponent, FText BossName);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void UpdateBossStatus(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void HideBossHPBar();

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|HUD")
	TObjectPtr<UMVPlayerStatusWidget> PlayerStatus;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|HUD")
	TObjectPtr<UMVQuickSlotWidget> HPSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|HUD")
	TObjectPtr<UMVQuickSlotWidget> StaminaSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|HUD")
	TObjectPtr<UMVCurrencyStatusWidget> CurrencyStatus;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|HUD")
	TObjectPtr<UMVBossHPBarWidget> BossHPBar;

private:
	void BuildNativeWidgetTree();
	void BindPlayerConsumableComponent(UMVPlayerConsumableComponent* ConsumableComponent);
	void ApplyHealingPotionQuickSlotView();

	UFUNCTION()
	void HandleHealingPotionStateChanged(const FMVHealingPotionRuntimeState& HealingPotionState);

	FText CachedBossName;

	UPROPERTY(Transient)
	TObjectPtr<UMVPlayerConsumableComponent> BoundPlayerConsumableComponent;
};
