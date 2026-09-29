#pragma once

#include "CoreMinimal.h"
#include "Struct/MVProgressionTypes.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "UI/Base/MVWindowBase.h"
#include "Input/UIActionBindingHandle.h"
#include "Input/Events.h"
#include "Input/NavigationReply.h"
#include "MVLevelUpWindow.generated.h"

class UMVCombatStateComponent;
class UMVStatComponent;
class UMVWorldStateSubsystem;
class UMVLevelUpAttributeEntryWidget;
class UMVLevelUpStatComparisonWidget;
class UMVLevelUpStatBlockWidget;
class UButton;
class UMVLevelUpSummaryWidget;
class APlayerController;
class UCommonActionWidget;

/**
 * 레벨업 창의 임시 투자·미리보기·확정·항목 선택과 입력 관리
 *
 * 임시 투자량·생성한 행·선택 순서는 이 창 소유
 * 확정 투자량·재화는 WorldState, 계산과 확정 검증은 ProgressionSubsystem 책임
 * Blueprint는 배치·행 클래스·선택 배경과 입력 안내 위젯 담당
 *
 * Construct에서 확인 입력·버튼 연결, Destruct에서 연결 해제
 * 활성화 시 상태 구독·행 생성·초점 확보
 * 비활성화 시 상태 구독·행 이벤트 해제, 선택 상태와 임시 투자 폐기
 * 사망·전투 진입·필드 전환 시 창 닫기
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVLevelUpWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVLevelUpWindow(const FObjectInitializer& ObjectInitializer);
	
	static bool CanOpenForPlayer(const APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	bool AdjustPendingRank(FGameplayTag ProgressionId, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|LevelUp")
	bool CanAdjustPendingRank(FGameplayTag ProgressionId, int32 Delta) const;
	
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	bool ConfirmLevelUp();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	void CancelLevelUp();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	void RefreshLevelUpPreview();

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|LevelUp")
	bool CanConfirmLevelUp() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|LevelUp")
	int32 GetPendingRank(FGameplayTag ProgressionId) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|LevelUp")
	TArray<FMVProgressionEntryDefinition> GetProgressionEntries() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FNavigationReply NativeOnNavigation(const FGeometry& InGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Maverick|UI|LevelUp")
	void BP_LevelUpPreviewChanged(const FMVProgressionEvaluation& Evaluation, bool bCanConfirm);

	UFUNCTION(BlueprintImplementableEvent, Category = "Maverick|UI|LevelUp")
	void BP_LevelUpCommitFinished(const FMVProgressionEvaluation& Evaluation);

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UMVLevelUpSummaryWidget> LevelAndCurrencyBlock;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;
	
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UMVLevelUpStatBlockWidget> AttributeBlock;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UMVLevelUpStatBlockWidget> ComparisonBlock;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> ConfirmInputHint;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> CancelInputHint;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	TSubclassOf<UMVLevelUpAttributeEntryWidget> AttributeEntryClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	TSubclassOf<UMVLevelUpStatComparisonWidget> ComparisonEntryClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	FMVProgressionEvaluation LevelUpPreview;

private:
	void RegisterLevelUpInputActions();
	void UnregisterLevelUpInputActions();
	void HandleConfirmInput(EInputEvent InputEvent);
	bool HandleLevelUpNavigation(EUINavigation Direction);
	void UpdateAttributeSelection();

	UFUNCTION()
	void HandleAttributeSelectionRequested(FGameplayTag ProgressionId);

	bool RebuildLevelUpRows();
	void ClearLevelUpRows();
	void UpdateLevelUpRows();
	void UpdateLevelUpSummary();

	UFUNCTION()
	void HandleConfirmClicked();
	
	bool BuildPendingRankCandidate(FGameplayTag ProgressionId, int32 Delta, TMap<FGameplayTag, int32>& OutCandidateRanks) const;

	UFUNCTION()
	void HandleRankAdjustmentRequested(FGameplayTag ProgressionId, int32 Delta);

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UMVLevelUpAttributeEntryWidget>> AttributeRows;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UMVLevelUpStatComparisonWidget>> ComparisonRows;

	
	void BindRuntimeEvents();
	void UnbindRuntimeEvents();
	bool IsInteractionAllowed() const;

	UFUNCTION()
	void HandleProgressionChanged(const FMVPlayerProgressionSaveData& PlayerProgression);

	UFUNCTION()
	void HandleBaseStatsReady(int32 Revision);

	UFUNCTION()
	void HandlePlayerDead();

	UFUNCTION()
	void HandleCombatStateChanged(bool bInCombat);

	UFUNCTION()
	void HandleTransitionPhaseChanged(EMVFieldTransitionPhase NewPhase);

	TMap<FGameplayTag, int32> PendingRanks;

	TWeakObjectPtr<UMVWorldStateSubsystem> WorldState;
	TWeakObjectPtr<UMVStatComponent> PlayerStats;
	TWeakObjectPtr<UMVCombatStateComponent> PlayerCombatState;
	TWeakObjectPtr<UMVFieldTransitionSubsystem> FieldTransition;

	int32 PreviewStatRevision = 0;
	bool bCommitInProgress = false;
	bool bLevelUpRowsReady = false;
	
	TArray<FUIActionBindingHandle> ConfirmInputBindings;
	TArray<FGameplayTag> OrderedAttributeIds;

	int32 SelectedAttributeIndex = INDEX_NONE;
	
};