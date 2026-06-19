#include "Animation/NotifyStates/MVAnimNotifyState_InputBuffer.h"

#include "Components/MVActionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVActionComponent* FindActionComponent(const USkeletalMeshComponent* MeshComp)
{
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVActionComponent>() : nullptr;
}
}

void UMVAnimNotifyState_InputBuffer::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVActionComponent* ActionComponent = FindActionComponent(MeshComp))
	{
		ActionComponent->BeginInputBufferWindow();
	}
}

void UMVAnimNotifyState_InputBuffer::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVActionComponent* ActionComponent = FindActionComponent(MeshComp))
	{
		ActionComponent->EndInputBufferWindow();
	}
}

FString UMVAnimNotifyState_InputBuffer::GetNotifyName_Implementation() const
{
	return TEXT("Input Buffer Window");
}
