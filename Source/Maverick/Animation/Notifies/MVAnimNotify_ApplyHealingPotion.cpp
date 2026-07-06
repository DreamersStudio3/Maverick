#include "Animation/Notifies/MVAnimNotify_ApplyHealingPotion.h"

#include "Character/PC/Consumable/MVPlayerConsumableComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
UMVPlayerConsumableComponent* FindApplyHealingPotionComponent(const USkeletalMeshComponent* MeshComp)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	return Owner ? Owner->FindComponentByClass<UMVPlayerConsumableComponent>() : nullptr;
}
}

void UMVAnimNotify_ApplyHealingPotion::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UMVPlayerConsumableComponent* ConsumableComponent = FindApplyHealingPotionComponent(MeshComp))
	{
		ConsumableComponent->ApplyHealingPotionEffect();
	}
}

FString UMVAnimNotify_ApplyHealingPotion::GetNotifyName_Implementation() const
{
	return TEXT("Apply Healing Potion");
}
