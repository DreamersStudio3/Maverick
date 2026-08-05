#include "Animation/NotifyStates/MVAnimNotifyState_AirborneLandDetector.h"

#include "Components/MVHitReactionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVHitReactionComponent* FindAirborneLandDetectorHitReactionComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVHitReactionComponent>() : nullptr;
}
}

void UMVAnimNotifyState_AirborneLandDetector::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVHitReactionComponent* HitReactionComponent = FindAirborneLandDetectorHitReactionComponent(MeshComp))
	{
		HitReactionComponent->BeginAirborneLandDetector();
	}
}

void UMVAnimNotifyState_AirborneLandDetector::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVHitReactionComponent* HitReactionComponent = FindAirborneLandDetectorHitReactionComponent(MeshComp))
	{
		HitReactionComponent->EndAirborneLandDetector();
	}
}

FString UMVAnimNotifyState_AirborneLandDetector::GetNotifyName_Implementation() const
{
	return TEXT("HitReaction Land Detector");
}
