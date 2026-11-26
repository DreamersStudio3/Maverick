#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "MVGameplayInputWidget.generated.h"

/**
 * UI 레이어의 기본 입력 설정 제공, 레이어에 포함된 동안 활성 상태 유지
 * 메뉴 창이 있거나 화면 전환 중이면 대체 입력도 메뉴 모드로 유지
 * 커서·이동·시점 차단의 실제 적용은 CommonUI에 위임
 */
UCLASS()
class MAVERICK_API UMVGameplayInputWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UMVGameplayInputWidget(const FObjectInitializer& ObjectInitializer);

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	void SetMenuInputBlocked(bool bBlocked);

protected:
	virtual void NativeOnInitialized() override;

private:
	bool bMenuInputBlocked = false;
};
