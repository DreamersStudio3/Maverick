#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MVAnimNotifyState_DodgeLaunch.generated.h"

class UCurveFloat;

UCLASS(meta = (DisplayName = "MV Dodge Launch"))
class MAVERICK_API UMVAnimNotifyState_DodgeLaunch : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Action|Dodge", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> DistanceCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Action|Dodge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DistanceScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Action|Dodge", meta = (AllowPrivateAccess = "true"))
	bool bApplyVerticalLaunchOnBegin = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Action|Dodge", meta = (AllowPrivateAccess = "true"))
	bool bClearHorizontalVelocityOnEnd = true;
};
