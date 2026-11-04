#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "MVGameplayInputWidget.generated.h"

/**
 * 메뉴가 없는 동안 사용할 게임 입력 설정 제공
 * UI 레이어에 포함되어 있는 동안 활성 상태 유지
 * 커서와 입력 잠금의 실제 전환은 CommonUI에 위임
 */
UCLASS()
class MAVERICK_API UMVGameplayInputWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UMVGameplayInputWidget(const FObjectInitializer& ObjectInitializer);

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	virtual void NativeOnInitialized() override;
};