#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MVAnimNotifyState_AirborneLandDetector.generated.h"

/**
 * HitReaction의 Land 전환을 감지하는 구간 표시.
 *
 * Airborne은 movement mode로 착지를 보고, KnockDown은 LaunchDuration이 끝나면 Land 섹션으로 점프한다.
 */
UCLASS(meta = (DisplayName = "MV HitReaction Land Detector"))
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
