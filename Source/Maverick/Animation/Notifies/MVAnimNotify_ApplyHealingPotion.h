#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_ApplyHealingPotion.generated.h"

/**
 * 회복약 사용 몽타주에서 실제 HP 회복과 카운트 소비가 확정되는 one-shot notify.
 *
 * UseConsumable 입력은 회복약 사용 액션 몽타주만 시작하고, 이 notify가 배치된 프레임에서
 * PlayerConsumableComponent가 HP 회복과 카운트 감소를 적용한다.
 */
UCLASS(meta = (DisplayName = "MV Apply Healing Potion"))
class MAVERICK_API UMVAnimNotify_ApplyHealingPotion : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
