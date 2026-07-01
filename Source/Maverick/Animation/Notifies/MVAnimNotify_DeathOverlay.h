#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_DeathOverlay.generated.h"

/**
 * 사망 몽타주에서 DeathOverlay 표시 프레임을 DeathComponent에 알리는 one-shot notify.
 *
 * 디졸브는 캐릭터 시각 효과 타이밍을 담당하고, 이 notify는 플레이어 사망 UI 표시 타이밍만 담당한다.
 * 사망 몽타주마다 dissolve 이후 원하는 프레임에 배치해 YOU DIED 표시 시점을 독립적으로 조절한다.
 */
UCLASS(meta = (DisplayName = "MV Death Overlay"))
class MAVERICK_API UMVAnimNotify_DeathOverlay : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
