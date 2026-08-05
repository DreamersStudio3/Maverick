#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_HitReactionDeathHandoff.generated.h"

/**
 * lethal HitReaction이 Lying 상태 표현으로 넘어가기 전 DeathComponent에 사망 handoff를 알린다.
 *
 * KD/AB 본 리액션이 끝난 직후, Lying 섹션으로 진입하기 전 프레임에 배치한다. HP가 0이 아니면
 * DeathComponent에 보류된 사망 문맥이 없으므로 아무 일도 하지 않고 기존 Lying 흐름이 유지된다.
 */
UCLASS(meta = (DisplayName = "MV HitReaction Death Handoff"))
class MAVERICK_API UMVAnimNotify_HitReactionDeathHandoff : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
