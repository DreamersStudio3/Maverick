#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "Struct/MVHitTypes.h"
#include "Struct/MVWeaponTypes.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "UI/Base/MVCommonButtonBase.h"
#include "UI/Base/MVWindowBase.h"
#include "MVWeaponPassiveSelectionWindow.generated.h"

class AMVPlayerCharacter;
class UButton;
class UImage;
class UScrollBox;
class UTextBlock;
class UTexture2D;
class UMVHUDWidgetBase;
class UCommonActionWidget;
struct FMVSkillDataTableColumn;
struct FMVWeaponTableRow;

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
 * 보유 무기 목록과 조회 중인 무기 종류의 패시브 후보를 표시한다.
 * 활성화 시 플레이어 사건을 구독하고 창이 닫히면 행과 구독을 정리한다.
 * 후보 이동은 미리보기만 갱신한다. 적용 버튼은 무기 컴포넌트에 교체 또는 해제를 요청한 뒤 실제 장착 상태를 다시 읽는다.
 * 활성화 동안 메인 HUD를 숨기며 종료할 때 같은 HUD의 이전 표시 상태를 복구한다.
 * 위젯 Blueprint는 창·행의 배치와 대체 그림을 제공한다.
 */
UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVWeaponPassiveSelectionWindow : public UMVWindowBase
{
	GENERATED_BODY()

public:
	static bool CanOpenForPlayer(const APlayerController* Controller);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|UI|Weapon Passive")
	TSubclassOf<UMVPassiveCandidateEntryWidget> CandidateEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|UI|Weapon Passive")
	TObjectPtr<UTexture2D> MissingArtwork;

private:
	void ReleasePreview();
	void BrowseWeapon(UMVPassiveCandidateEntryWidget* Entry, FGameplayTag ItemTag, FGameplayTag WeaponType, bool bFocusCandidates);
	void RebuildCandidates(bool bFocusCandidates = true);
	void AddEntry(UMVPassiveCandidateEntryWidget* Entry);
	void PreviewCandidate(UMVPassiveCandidateEntryWidget* Entry);
	void UpdateNavigation();
	void RegisterConfirmInput();
	void HandleConfirmInput(EInputEvent InputEvent);
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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RequestMessage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> ConfirmInputHint;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> CancelInputHint;

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
