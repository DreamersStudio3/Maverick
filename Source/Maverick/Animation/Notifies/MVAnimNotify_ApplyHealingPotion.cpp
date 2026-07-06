#include "Animation/Notifies/MVAnimNotify_ApplyHealingPotion.h"

#include "Character/PC/Consumable/MVPlayerConsumable.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVPlayerConsumable* FindApplyHealingPotionSubmodule(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	AMVPlayerCharacter* PlayerCharacter = Owner ? Cast<AMVPlayerCharacter>(Owner) : nullptr;
	return PlayerCharacter ? PlayerCharacter->PlayerConsumable : nullptr;
}
}

void UMVAnimNotify_ApplyHealingPotion::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UMVPlayerConsumable* Consumable = FindApplyHealingPotionSubmodule(MeshComp))
	{
		Consumable->ApplyHealingPotionEffect();
	}
}

FString UMVAnimNotify_ApplyHealingPotion::GetNotifyName_Implementation() const
{
	return TEXT("Apply Healing Potion");
}
