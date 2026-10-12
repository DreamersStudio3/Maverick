#include "Combat/Duelist/MVAnimNotify_DuelistFireChain.h"

#include "Combat/Duelist/MVDuelistChainPullAbility.h"
#include "Components/MVCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UMVAnimNotify_DuelistFireChain::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	const UMVCombatComponent* Combat = Owner ? Owner->FindComponentByClass<UMVCombatComponent>() : nullptr;
	UMVDuelistChainPullAbility* Ability = Combat ? Cast<UMVDuelistChainPullAbility>(Combat->CurrentAbilityInstance) : nullptr;
	if (Ability) Ability->FireChain();
}
