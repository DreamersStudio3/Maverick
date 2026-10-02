#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVLevelUpStatNameWidget.generated.h"

class UTextBlock;

/**
 * 투자 행과 비교 행에서 공유하는 스탯 이름 표시.
 *
 * 상위 행에서 전달한 이름만 표시하며 캐릭터 상태 조회와 투자 처리 책임 제외.
 * Construct 이전 이름 설정과 위젯 재생성 후 표시 복원 지원.
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVLevelUpStatNameWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVLevelUpStatNameWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|LevelUp")
	void SetStatName(const FText& InDisplayName);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|UI|LevelUp", meta = (BindWidget))
	TObjectPtr<UTextBlock> StatName;

private:
	void ApplyDisplay();

	FText CachedDisplayName;
	bool bHasDisplayName = false;
};