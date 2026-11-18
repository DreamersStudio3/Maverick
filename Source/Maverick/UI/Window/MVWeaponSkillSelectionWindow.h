#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "Struct/MVHitTypes.h"
#include "Struct/MVWeaponTypes.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "UI/Base/MVCommonButtonBase.h"
#include "UI/Base/MVWindowBase.h"
#include "Components/Button.h"
#include "Weapon/Skill/MVSkillSelectionTypes.h"
#include "MVWeaponSkillSelectionWindow.generated.h"

class AMVPlayerCharacter;
class UButton;
class UImage;
class UScrollBox;
class UTextBlock;
class UTexture2D;
class UMVHUDWidgetBase;
class UCommonActionWidget;
class UHorizontalBox;
class UWidgetTree;
struct FMVSkillDataTableColumn;
struct FMVWeaponTableRow;

DECLARE_MULTICAST_DELEGATE_OneParam(FMVOnSkillTabChosen, EMVSkillSlot);

/** 슬롯 정의로 만든 탭 버튼. 선택과 그림 표시만 담당하며 게임 상태를 소유하지 않는다. */
UCLASS()
class MAVERICK_API UMVSkillSlotTabButton : public UButton
{
	GENERATED_BODY()
public:
	void InitializeSlot(UWidgetTree& Tree, const FMVSkillSlotDefinition& Definition);
	void SetSlotSelected(bool bSelected);
	void SetSlotIcon(const TSoftObjectPtr<UTexture2D>& Icon, UTexture2D* Fallback);
	EMVSkillSlot GetSkillSlot() const { return SkillSlotValue; }
	FMVOnSkillTabChosen OnSlotChosen;
private:
	UFUNCTION() void HandleClicked();
	EMVSkillSlot SkillSlotValue = EMVSkillSlot::NONE;
	UPROPERTY(Transient) TObjectPtr<UImage> IconImage;
};

/** 두 목록에서 공유하는 행. 무기 이름·장착 표시 또는 패시브 후보·해제 대상을 표시한다. */
UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVPassiveCandidateEntryWidget : public UMVCommonButtonBase
{
	GENERATED_BODY()

public:
	void SetWeapon(const FMVWeaponTableRow& Weapon, bool bEquipped, UTexture2D* MissingArtwork);
	void SetCandidate(const FDataTableRowHandle& InHandle, const FMVSkillDataTableColumn& Data,
		const FText& State, bool bInCanApply, UTexture2D* MissingArtwork);
	void SetClearSelection(const FDataTableRowHandle& Selected);
	void SetPreviewed(bool bPreviewed);
	bool IsClearSelection() const { return bClearSelection; }
	bool CanApply() const { return bCanApply; }
	const FDataTableRowHandle& GetSkillRow() const { return SkillRow; }

private:
	bool bClearSelection = false;
	bool bCanApply = false;

	UPROPERTY(Transient)
	FDataTableRowHandle SkillRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CandidateName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CandidateIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CandidateState;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PreviewMarker;
};

/**
 * 보유 무기와 패시브·QWER 후보를 한 창에서 조회하고 선택·저장 재시도를 요청한다.
 * 활성화 시 플레이어 사건을 구독하고 창이 닫히면 행과 구독을 정리한다.
 * 후보 이동은 미리보기만 갱신한다. 적용 버튼은 무기 컴포넌트에 교체 또는 해제를 요청한 뒤 실제 장착 상태를 다시 읽는다.
 * 활성화 동안 메인 HUD를 숨기며 종료할 때 같은 HUD의 이전 표시 상태를 복구한다.
 * 위젯 Blueprint는 창·행의 배치와 대체 그림을 제공한다.
 */
UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVWeaponSkillSelectionWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	UMVWeaponSkillSelectionWindow(const FObjectInitializer& ObjectInitializer);
	static bool CanOpenForPlayer(const APlayerController* Controller);
	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Skill Selection")
	void SelectSkillSlot(EMVSkillSlot InSkillSlot);
	UFUNCTION(BlueprintPure, Category = "Maverick|UI|Skill Selection")
	EMVSkillSlot GetViewedSkillSlot() const { return ViewedSlot; }

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|UI|Weapon Passive")
	TSubclassOf<UMVPassiveCandidateEntryWidget> CandidateEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|UI|Weapon Passive")
	TObjectPtr<UTexture2D> MissingArtwork;

private:
	FMVSkillSelectionContext MakeSelectionContext() const;
	void BuildSlotTabs();
	void RefreshSlotTabs();
	void HandleSkillSelectionChanged(const FMVSkillSelectionContext& Scope);
	void ShowSelectionResult(const FMVSkillSelectionResult& Result);
	void ShowSaveResult(EMVSkillSelectionSaveStatus Status);
	UFUNCTION() void RetrySave();
	void ReleasePreview();
	void BrowseWeapon(UMVPassiveCandidateEntryWidget* Entry, FGameplayTag ItemTag, FGameplayTag WeaponType, bool bFocusCandidates);
	void RebuildCandidates(bool bFocusCandidates = true);
	void AddEntry(UMVPassiveCandidateEntryWidget* Entry);
	void PreviewCandidate(UMVPassiveCandidateEntryWidget* Entry);
	void UpdateNavigation();
	void RegisterConfirmInput();
	void RefreshInputHints();
	void HandleConfirmInput(EInputEvent InputEvent);
	bool IsConfirmKey(FKey InKey) const;
	void SetRequestMessage(const FText& Message);

	UFUNCTION()
	void QueueRefresh();

	UFUNCTION()
	void ApplyPreviewedCandidate();

	UFUNCTION()
	void RefreshWeapons();

	UFUNCTION()
	void OnEquippedWeaponChanged(const FMVEquippedWeaponState& Weapon);

	UFUNCTION()
	void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void OnTransitionChanged(EMVFieldTransitionPhase Phase);

	UFUNCTION()
	void OnPlayerActionStarted(FName TableName, FName RowName);

	UFUNCTION()
	void OnPlayerDamageApplied(float AppliedDamage, float PreviousHP, float CurrentHP, const FMVResolvedHitData& HitData);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> WeaponList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> CandidateList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PreviewName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PreviewDescription;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PreviewImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RequestMessage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> ConfirmInputHint;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> CancelInputHint;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CancelInputLabel;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UHorizontalBox> SlotTabs;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> SharingScopeText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> RetrySaveButton;
	UPROPERTY(Transient) TArray<TObjectPtr<UMVSkillSlotTabButton>> SlotButtons;
	EMVSkillSlot ViewedSlot = EMVSkillSlot::PassiveSkill;
	FDelegateHandle SkillChangedHandle;
	EMVSkillSelectionSaveStatus LastSaveStatus = EMVSkillSelectionSaveStatus::NotRequested;

	TWeakObjectPtr<AMVPlayerCharacter> ViewedPlayer;
	TWeakObjectPtr<UMVPassiveCandidateEntryWidget> ViewedWeaponEntry;
	FGameplayTag ViewedItemTag;
	FGameplayTag ViewedWeaponType;
	TWeakObjectPtr<UMVPassiveCandidateEntryWidget> PreviewedEntry;
	TWeakObjectPtr<UMVHUDWidgetBase> SuppressedHUD;
	ESlateVisibility PreviousHUDVisibility = ESlateVisibility::Collapsed;
	TArray<FUIActionBindingHandle> ConfirmInputBindings;
	FTimerHandle RefreshTimer;
	bool bApplyingSelection = false;
};
