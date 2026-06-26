#include "Animation/Notifies/MVAnimNotify_HitReactionDefaultRecovery.h"

#include "Components/MVHitReactionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVHitReactionComponent* FindHitReactionDefaultRecoveryComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVHitReactionComponent>() : nullptr;
}
}

void UMVAnimNotify_HitReactionDefaultRecovery::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UMVHitReactionComponent* HitReactionComponent = FindHitReactionDefaultRecoveryComponent(MeshComp))
	{
		HitReactionComponent->RequestDefaultRecoveryAction();
	}
}

FString UMVAnimNotify_HitReactionDefaultRecovery::GetNotifyName_Implementation() const
{
	return TEXT("HitReaction Default Recovery");
}
