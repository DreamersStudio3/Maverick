#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MVAnimNotifyState_AirborneLandDetector.generated.h"

/**
 * Airborne 피격 리액션의 착지 감지 구간을 표시한다.
 *
 * NotifyState가 열린 동안 HitReactionComponent는 캐릭터 movement mode를 감시하고,
 * 착지 조건이 만족되면 현재 몽타주를 Land 섹션으로 점프시킨다.
 */
UCLASS(meta = (DisplayName = "MV Airborne Land Detector"))
class MAVERICK_API UMVAnimNotifyState_AirborneLandDetector : public UAnimNotifyState
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
