#include "Animation/Notifies/MVAnimNotify_HitReactionDeathHandoff.h"

#include "Components/MVDeathComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVDeathComponent* FindHitReactionDeathHandoffComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVDeathComponent>() : nullptr;
}
}

void UMVAnimNotify_HitReactionDeathHandoff::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UMVDeathComponent* DeathComponent = FindHitReactionDeathHandoffComponent(MeshComp))
	{
		DeathComponent->NotifyHitReactionDeathHandoff();
	}
}

FString UMVAnimNotify_HitReactionDeathHandoff::GetNotifyName_Implementation() const
{
	return TEXT("HitReaction Death Handoff");
}
