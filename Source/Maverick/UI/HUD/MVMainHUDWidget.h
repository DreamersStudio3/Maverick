#pragma once

#include "CoreMinimal.h"
#include "Struct/MVHealingPotionTypes.h"
#include "Struct/MVProgressionTypes.h"
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
class UMVWorldStateSubsystem;
class UTextBlock;

/**
 * 플레이어 상태·소모품·스킬·재화·보스 표시를 연결하는 메인 HUD
 *
 * 하위 표시 위젯에 각 도메인의 현재 상태 전달
 * 재화 원본과 저장은 WorldState 책임, Blueprint는 HUD 배치 담당
 * Construct에서 재화 변경 구독과 초기 표시 갱신
 * Destruct에서 재화·소모품 연결 해제
 */
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

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void MarkBossPatrolComplete();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Boss")
	void MarkBossPlayStarted();


	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Patrol;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlayInfo;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
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
	void BindWorldState();
	void UnbindWorldState();
	void RefreshCurrencyHUD();

	UFUNCTION()
	void HandlePlayerProgressionChanged(const FMVPlayerProgressionSaveData& PlayerProgression);
	
	UFUNCTION()
	void HandleHealingPotionStateChanged(const FMVHealingPotionRuntimeState& HealingPotionState);

	FText CachedBossName;

	UPROPERTY(Transient)
	TObjectPtr<UMVPlayerConsumable> BoundPlayerConsumable;
	
	TWeakObjectPtr<UMVWorldStateSubsystem> BoundWorldState;
};
