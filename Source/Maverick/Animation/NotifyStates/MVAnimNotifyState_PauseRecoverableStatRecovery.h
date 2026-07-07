#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MVAnimNotifyState_PauseRecoverableStatRecovery.generated.h"

/**
 * 애니메이션 구간 동안 회복 가능한 스탯의 자연 회복을 일시정지한다.
 *
 * Ability 활성 구간과 별개로 차지 시작, 공격 후딜 종료처럼 스태미너 회복을 막아야 하는 정확한
 * 애니메이션 구간에 배치한다. Begin/End는 StatComponent의 pause count를 사용하므로 중첩 배치가 가능하다.
 */
UCLASS(meta = (DisplayName = "MV Pause Recoverable Stat Recovery"))
class MAVERICK_API UMVAnimNotifyState_PauseRecoverableStatRecovery : public UAnimNotifyState
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
