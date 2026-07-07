#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MVAnimNotify_CooldownReadyActionCut.generated.h"

/**
 * Cuts the current action at an animation-authored branch point when a cooldown is ready.
 *
 * The notify does not choose or start the follow-up action. It only ends the
 * current ActionComponent action so StateTree can evaluate the next state.
 */
UCLASS(meta = (DisplayName = "MV Cooldown Ready Action Cut"))
class MAVERICK_API UMVAnimNotify_CooldownReadyActionCut : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Cooldown")
	FName CooldownActionId = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Action", meta = (ClampMin = "0.0"))
	float BlendOutTime = 0.2f;
};
