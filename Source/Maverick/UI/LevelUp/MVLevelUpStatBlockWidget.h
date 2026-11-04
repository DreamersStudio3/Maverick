#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVLevelUpStatBlockWidget.generated.h"

class UVerticalBox;
class UMVLevelUpAttributeEntryWidget;
class UMVLevelUpStatComparisonWidget;

/**
 * 레벨업 투자 행과 스탯 비교 행을 세로로 배치하는 컨테이너
 *
 * 상위 창의 요청에 따라 행 생성과 제거만 수행
 * 성장 계산·수치 전달·행 이벤트 연결은 상위 창 책임
 * 행 수와 스탯 종류를 고정하지 않으며 디자이너에서 행 생성 제외
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVLevelUpStatBlockWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVLevelUpStatBlockWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	void ClearEntries();

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	UMVLevelUpAttributeEntryWidget* AddAttributeEntry(TSubclassOf<UMVLevelUpAttributeEntryWidget> EntryClass);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	UMVLevelUpStatComparisonWidget* AddComparisonEntry(TSubclassOf<UMVLevelUpStatComparisonWidget> EntryClass);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UVerticalBox> StatBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|UI|LevelUp")
	FMargin EntryPadding = FMargin(0.0f, 0.0f, 0.0f, 2.0f);

private:
	UUserWidget* CreateEntry(TSubclassOf<UUserWidget> EntryClass);
};