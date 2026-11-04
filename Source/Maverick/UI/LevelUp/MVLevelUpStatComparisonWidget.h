#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVLevelUpStatComparisonWidget.generated.h"

class UTextBlock;
class UMVLevelUpStatNameWidget;

/**
 * 스탯 하나의 이름·현재값·예상값과 변경 방향 표시
 *
 * 상위 창에서 전달한 값만 표시하며 캐릭터 조회·성장 계산·저장 책임 제외
 * Construct 이전 값 설정과 위젯 재생성 후 표시 복원 지원
 * 낮을수록 유리한 스탯도 개선 방향 설정으로 표시 지원
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVLevelUpStatComparisonWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVLevelUpStatComparisonWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	bool SetComparison(
		FGameplayTag InStatId,
		const FText& InDisplayName,
		float InCurrentValue,
		float InPreviewValue,
		bool bInLowerIsBetter = false);

	UFUNCTION(BlueprintPure, Category = "Maverick|UI|LevelUp")
	FGameplayTag GetStatId() const { return StatId; }

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UMVLevelUpStatNameWidget> StatNameWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> StatValueOld;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> StatValueNew;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	FLinearColor UnchangedValueColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	FLinearColor ImprovedValueColor = FLinearColor(0.35f, 0.85f, 0.45f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	FLinearColor ReducedValueColor = FLinearColor(0.9f, 0.3f, 0.3f, 1.0f);

private:
	void ApplyDisplay();

	FGameplayTag StatId;
	FText CachedDisplayName;

	float CurrentValue = 0.0f;
	float PreviewValue = 0.0f;

	bool bLowerIsBetter = false;
	bool bHasComparison = false;
};