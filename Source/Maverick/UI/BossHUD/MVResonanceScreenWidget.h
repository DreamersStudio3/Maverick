#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MVResonanceScreenWidget.generated.h"

/** 공명 폭주 경고·잔여 시간·강인도 표시, 입력 미소비; 실패 암전은 보스 Task 종료와 독립된 타이머로 해제 */
UCLASS()
class MAVERICK_API UMVResonanceScreenWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	float Remaining = 5.0f;
	float Poise = 40.0f;
	float Duration = 5.0f;
	bool bBlackout = false;
	void StartBlackout(float Seconds);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
private:
	FTimerHandle BlackoutTimer;
};
