#include "Animation/NotifyStates/MVAnimNotifyState_BlockMovementInput.h"

#include "Character/MVCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
AMVCharacterBase* FindBlockMovementInputCharacter(const USkeletalMeshComponent* MeshComp)
{
	return MeshComp ? Cast<AMVCharacterBase>(MeshComp->GetOwner()) : nullptr;
}
}

void UMVAnimNotifyState_BlockMovementInput::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AMVCharacterBase* Character = FindBlockMovementInputCharacter(MeshComp))
	{
		Character->BeginMovementInputBlock();
	}
}

void UMVAnimNotifyState_BlockMovementInput::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AMVCharacterBase* Character = FindBlockMovementInputCharacter(MeshComp))
	{
		Character->EndMovementInputBlock();
	}
}

FString UMVAnimNotifyState_BlockMovementInput::GetNotifyName_Implementation() const
{
	return TEXT("Block Movement Input");
}
