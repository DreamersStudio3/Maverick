#include "Animation/NotifyStates/MVAnimNotifyState_Invincible.h"

#include "Character/MVCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
AMVCharacterBase* FindInvincibleCharacter(const USkeletalMeshComponent* MeshComp)
{
	return MeshComp ? Cast<AMVCharacterBase>(MeshComp->GetOwner()) : nullptr;
}
}

void UMVAnimNotifyState_Invincible::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AMVCharacterBase* Character = FindInvincibleCharacter(MeshComp))
	{
		Character->BeginInvincibility();
	}
}

void UMVAnimNotifyState_Invincible::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AMVCharacterBase* Character = FindInvincibleCharacter(MeshComp))
	{
		Character->EndInvincibility();
	}
}

FString UMVAnimNotifyState_Invincible::GetNotifyName_Implementation() const
{
	return TEXT("Invincible");
}
