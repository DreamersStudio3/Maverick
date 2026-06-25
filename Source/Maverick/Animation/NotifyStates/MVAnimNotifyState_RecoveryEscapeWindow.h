#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MVAnimNotifyState_RecoveryEscapeWindow.generated.h"

/**
 * 현재 액션 리커버리/팔로쓰루 구간에서 입력을 소비할 수 있는 window를 연다.
 *
 * 이 NotifyState가 열린 동안 입력된 액션은 buffered action 슬롯에 들어갈 수 있고, HitReaction/Combat 같은
 * 도메인 컴포넌트는 InputManagerComponent의 입력 이벤트를 소비해 현재 몽타주의 후속 처리를 요청할 수 있다.
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
