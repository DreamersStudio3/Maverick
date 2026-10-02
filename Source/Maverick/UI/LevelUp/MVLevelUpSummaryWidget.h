#pragma once

#include "CoreMinimal.h"
#include "Struct/MVProgressionTypes.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVLevelUpSummaryWidget.generated.h"

class UTextBlock;

/**
 * 레벨업 전후 레벨·재화와 선택한 투자의 총비용 표시
 *
 * 상위 창이 전달한 평가 결과만 표시하며 계산·결제·저장 책임 제외
 * 재화와 비용은 int64 값을 그대로 표시
 * Construct 이전 값 설정과 위젯 재생성 후 표시 복원 지원
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVLevelUpSummaryWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVLevelUpSummaryWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	void SetSummary(const FMVProgressionEvaluation& Evaluation);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> Level_Current;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> Level_New;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrencyCurrent;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrencyNew;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrencyRequired;

private:
	void ApplyDisplay();

	UPROPERTY(Transient)
	FMVProgressionEvaluation CachedEvaluation;

	bool bHasSummary = false;
};