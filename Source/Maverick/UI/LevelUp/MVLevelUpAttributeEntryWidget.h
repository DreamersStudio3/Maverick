#pragma once

#include "CoreMinimal.h"
#include "Struct/MVProgressionTypes.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVLevelUpAttributeEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UMVLevelUpStatNameWidget;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnLevelUpRankAdjustmentRequested, FGameplayTag, ProgressionId, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnLevelUpAttributeSelectionRequested, FGameplayTag, ProgressionId);

/**
 * 성장 항목의 이름·확정 투자량·예상 투자량·증감 버튼·선택 상태 표시
 *
 * 상위 창에서 받은 표시 상태를 적용하고 선택·투자 변경 요청만 전달
 * 실제 투자량 관리·계산·저장 변경은 상위 창과 성장 서브시스템 책임
 * Construct에서 버튼 이벤트 연결, Destruct에서 연결 해제
 * 마우스 진입 시 선택 요청, 게임패드 가상 커서의 진입은 선택 요청에서 제외
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVLevelUpAttributeEntryWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVLevelUpAttributeEntryWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	void SetEntry(
		const FMVProgressionEntryDefinition& Entry,
		int32 InCurrentRank,
		int32 InPreviewRank,
		bool bInCanDecrease,
		bool bInCanIncrease);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|LevelUp")
	FMVOnLevelUpRankAdjustmentRequested OnRankAdjustmentRequested;

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	void SetSelected(bool bInSelected);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|UI|LevelUp")
	FMVOnLevelUpAttributeSelectionRequested OnSelectionRequested;
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> SelectionBorder;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	FLinearColor SelectedBackgroundColor = FLinearColor(0.35f, 0.30f, 0.20f, 0.65f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	FLinearColor UnselectedBackgroundColor = FLinearColor::Transparent;
	
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UMVLevelUpStatNameWidget> StatNameWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> StatValueOld;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> StatValueNew;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UButton> ButtonDecrease;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UButton> ButtonIncrease;

private:
	void ApplyDisplay();

	void ApplySelection();
	
	UFUNCTION()
	void HandleDecreaseClicked();

	UFUNCTION()
	void HandleIncreaseClicked();

	FGameplayTag ProgressionId;
	FText CachedDisplayName;

	int32 CurrentRank = 0;
	int32 PreviewRank = 0;

	bool bCanDecrease = false;
	bool bCanIncrease = false;
	bool bHasEntry = false;
	bool bSelected = false;
};