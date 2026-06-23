#include "Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.h"

#include "Components/MVActionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVActionComponent* FindBlockMovementInputActionComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVActionComponent>() : nullptr;
}
}

void UMVAnimNotifyState_BlockMovementInput::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVActionComponent* ActionComponent = FindBlockMovementInputActionComponent(MeshComp))
	{
		ActionComponent->BeginMovementInputBlock();
	}
}

void UMVAnimNotifyState_BlockMovementInput::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVActionComponent* ActionComponent = FindBlockMovementInputActionComponent(MeshComp))
	{
		ActionComponent->EndMovementInputBlock();
	}
}

FString UMVAnimNotifyState_BlockMovementInput::GetNotifyName_Implementation() const
{
	return TEXT("Block Movement Input");
}
