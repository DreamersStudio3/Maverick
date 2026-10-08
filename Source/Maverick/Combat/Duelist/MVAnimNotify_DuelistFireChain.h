#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_DuelistFireChain.generated.h"

UCLASS(meta = (DisplayName = "MV Duelist Fire Chain"))
class MAVERICK_API UMVAnimNotify_DuelistFireChain : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
