#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_HitReactionDefaultRecovery.generated.h"

/**
 * HitReaction 본 리액션에서 별도 Getup recovery 액션으로 넘어가는 handoff 지점을 표시한다.
 *
 * RecoveryEscapeWindow는 입력으로 현재 리액션을 끊을 수 있는 구간만 표현하고,
 * 입력이 없을 때의 기본 Getup 전환은 이 one-shot Notify가 배치된 프레임에서 요청한다.
 */
UCLASS(meta = (DisplayName = "MV HitReaction Default Recovery"))
class MAVERICK_API UMVAnimNotify_HitReactionDefaultRecovery : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
