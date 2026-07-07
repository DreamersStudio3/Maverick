#include "Animation/NotifyStates/MVAnimNotifyState_Ability.h"

#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interface/MVAbilityInterface.h"

namespace
{
UMVCombatComponent* FindAbilityNotifyCombatComponent(const USkeletalMeshComponent* MeshComp)
{
	const AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	return OwnerActor ? OwnerActor->FindComponentByClass<UMVCombatComponent>() : nullptr;
}
}

void UMVAnimNotifyState_Ability::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UMVCombatComponent* CombatComponent = FindAbilityNotifyCombatComponent(MeshComp);
	UMVAbilityBase* CurrentAbility = CombatComponent ? CombatComponent->CurrentAbilityInstance.Get() : nullptr;
	if (!CurrentAbility || (AbilityClass && !CurrentAbility->IsA(AbilityClass)))
	{
		return;
	}

	IMVAbilityInterface::Execute_StartAbility(CurrentAbility);
}

void UMVAnimNotifyState_Ability::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UMVCombatComponent* CombatComponent = FindAbilityNotifyCombatComponent(MeshComp);
	UMVAbilityBase* CurrentAbility = CombatComponent ? CombatComponent->CurrentAbilityInstance.Get() : nullptr;
	if (!CurrentAbility || (AbilityClass && !CurrentAbility->IsA(AbilityClass)))
	{
		return;
	}

	IMVAbilityInterface::Execute_EndAbility(CurrentAbility);
}

FString UMVAnimNotifyState_Ability::GetNotifyName_Implementation() const
{
	return TEXT("Activate Ability");
}
