#include "Animation/Notifies/MVAnimNotify_DeathOverlay.h"

#include "Components/MVDeathComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UMVAnimNotify_DeathOverlay::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UMVDeathComponent* DeathComponent = MeshComp && MeshComp->GetOwner()
		? MeshComp->GetOwner()->FindComponentByClass<UMVDeathComponent>()
		: nullptr;
	if (DeathComponent)
	{
		DeathComponent->NotifyDeathOverlayRequested();
	}
}

FString UMVAnimNotify_DeathOverlay::GetNotifyName_Implementation() const
{
	return TEXT("Death Overlay");
}
