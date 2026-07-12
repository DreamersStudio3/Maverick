#pragma once

#include "CoreMinimal.h"
#include "Struct/MVHealingPotionTypes.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "MVMainHUDWidget.generated.h"

class UMVBossHPBarWidget;
class UMVCombatComponent;
class UMVCurrencyStatusWidget;
class UMVPlayerConsumable;
class UMVPlayerSkillHUDWidget;
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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Maverick|UI|HUD")
	TObjectPtr<UMVPlayerSkillHUDWidget> PlayerSkillHUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|UI|HUD|Skill")
	FVector2D PlayerSkillHUDCanvasOffset = FVector2D(0.0f, -36.0f);

private:
	void BuildNativeWidgetTree();
	void EnsurePlayerSkillHUD();
	void BindPlayerConsumable(UMVPlayerConsumable* Consumable);
	void ApplyHealingPotionQuickSlotView();

	UFUNCTION()
	void HandleHealingPotionStateChanged(const FMVHealingPotionRuntimeState& HealingPotionState);

	FText CachedBossName;

	UPROPERTY(Transient)
	TObjectPtr<UMVPlayerConsumable> BoundPlayerConsumable;
};
