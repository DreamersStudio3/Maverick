#include "Animation/NotifyStates/MVAnimNotifyState_PauseRecoverableStatRecovery.h"

#include "Components/MVStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVStatComponent* FindPauseRecoverableStatRecoveryStatComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVStatComponent>() : nullptr;
}
}

void UMVAnimNotifyState_PauseRecoverableStatRecovery::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVStatComponent* StatComponent = FindPauseRecoverableStatRecoveryStatComponent(MeshComp))
	{
		StatComponent->BeginRecoverableStatRecoveryPause();
	}
}

void UMVAnimNotifyState_PauseRecoverableStatRecovery::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVStatComponent* StatComponent = FindPauseRecoverableStatRecoveryStatComponent(MeshComp))
	{
		StatComponent->EndRecoverableStatRecoveryPause();
	}
}

FString UMVAnimNotifyState_PauseRecoverableStatRecovery::GetNotifyName_Implementation() const
{
	return TEXT("Pause Recoverable Stat Recovery");
}
