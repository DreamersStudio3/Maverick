#include "Animation/Notifies/MVAnimNotify_DeathDissolve.h"

#include "Components/MVDeathComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UMVAnimNotify_DeathDissolve::Notify(
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
		DeathComponent->NotifyDeathDissolveStarted();
	}
}

FString UMVAnimNotify_DeathDissolve::GetNotifyName_Implementation() const
{
	return TEXT("Death Dissolve");
}
