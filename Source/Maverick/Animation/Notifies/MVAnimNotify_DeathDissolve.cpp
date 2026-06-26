#include "Animation/Notifies/MVAnimNotify_DeathDissolve.h"

#include "Components/SkeletalMeshComponent.h"
#include "System/MVRespawnSubsystem.h"

void UMVAnimNotify_DeathDissolve::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UMVRespawnSubsystem* RespawnSubsystem = UMVRespawnSubsystem::Get(MeshComp))
	{
		RespawnSubsystem->NotifyDeathDissolveStarted();
	}
}

FString UMVAnimNotify_DeathDissolve::GetNotifyName_Implementation() const
{
	return TEXT("Death Dissolve");
}
