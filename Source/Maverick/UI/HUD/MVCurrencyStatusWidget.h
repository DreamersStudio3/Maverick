#pragma once

#include "CoreMinimal.h"
#include "UI/Base/MVWidgetBase.h"
#include "MVCurrencyStatusWidget.generated.h"

class UTextBlock;

/**
 * HUD에 확정된 재화 수치를 표시하는 위젯
 *
 * 전달받은 int64 값을 보관하고 생성·재생성 시 표시 복원
 * 재화 읽기·변경 알림 구독은 상위 메인 HUD 책임
 * Blueprint는 CurrencyText 배치와 스타일 담당
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVCurrencyStatusWidget : public UMVWidgetBase
{
	GENERATED_BODY()

public:
	UMVCurrencyStatusWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Maverick|UI|Currency")
	void SetCurrency(int64 NewCurrency);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Maverick|UI|Currency")
	TObjectPtr<UTextBlock> CurrencyText;

private:
	void ApplyCurrencyDisplay();

	int64 CachedCurrency = 0;
};