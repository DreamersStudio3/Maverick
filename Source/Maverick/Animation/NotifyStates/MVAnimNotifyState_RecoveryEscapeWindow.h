#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MVAnimNotifyState_RecoveryEscapeWindow.generated.h"

/**
 * 현재 액션 후딜에서 buffered action으로 탈출할 수 있는 구간을 연다.
 *
 * 이 NotifyState가 열린 동안 입력된 액션은 buffered action 슬롯에 들어갈 수 있고,
 * ActionComponent는 이동 차단과 input buffer가 모두 닫힌 뒤에만 현재 몽타주를 끊는다.
 */
UCLASS(meta = (DisplayName = "MV Recovery Escape Window"))
class MAVERICK_API UMVAnimNotifyState_RecoveryEscapeWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
