#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_DeathDissolve.generated.h"

/**
 * 사망 몽타주에서 디졸브 시작 프레임을 DeathComponent에 알리는 one-shot notify.
 *
 * 실제 머티리얼 디졸브 컴포넌트가 추가되기 전까지는 actor-local 사망 시각 효과 cue 역할만 한다.
 * DeathOverlay 표시 타이밍은 `MV Death Overlay` notify가 별도로 담당한다.
 */
UCLASS(meta = (DisplayName = "MV Death Dissolve"))
class MAVERICK_API UMVAnimNotify_DeathDissolve : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
