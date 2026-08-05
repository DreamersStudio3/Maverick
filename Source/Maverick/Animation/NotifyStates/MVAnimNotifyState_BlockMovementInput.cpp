#include "Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.h"

#include "Components/MVInputManagerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVInputManagerComponent* FindBlockMovementInputInputManagerComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVInputManagerComponent>() : nullptr;
}
}

void UMVAnimNotifyState_BlockMovementInput::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UMVInputManagerComponent* InputManager = FindBlockMovementInputInputManagerComponent(MeshComp))
	{
		InputManager->BeginMovementInputBlock();
	}
}

void UMVAnimNotifyState_BlockMovementInput::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UMVInputManagerComponent* InputManager = FindBlockMovementInputInputManagerComponent(MeshComp))
	{
		InputManager->EndMovementInputBlock();
	}
}

FString UMVAnimNotifyState_BlockMovementInput::GetNotifyName_Implementation() const
{
	return TEXT("Block Movement Input");
}
