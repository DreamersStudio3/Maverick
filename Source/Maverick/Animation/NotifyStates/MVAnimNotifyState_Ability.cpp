// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/NotifyStates/MVAnimNotifyState_Ability.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"

void UMVAnimNotifyState_Ability::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	
	UMVCombatComponent* OwnerComp = OwnerActor->FindComponentByClass<UMVCombatComponent>();
	if (!OwnerComp)
	{
		return;
	}
	if (!AbilityClass)
	{
		return;
	}

	FMVSkillActionStruct* FindResult = OwnerComp->SkillMap.Find(AbilityClass);


	if (FindResult)
	{
		IMVAbilityInterface::Execute_StartAbility(FindResult->AbilityInstance);
	}

}

void UMVAnimNotifyState_Ability::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UMVCombatComponent* OwnerComp = OwnerActor->FindComponentByClass<UMVCombatComponent>();
	if (!OwnerComp)
	{
		return;
	}
	if (!AbilityClass)
	{
		return;
	}

	FMVSkillActionStruct* FindResult = OwnerComp->SkillMap.Find(AbilityClass);
	
	if (FindResult)
	{
		IMVAbilityInterface::Execute_EndAbility(FindResult->AbilityInstance);
	}
}

FString UMVAnimNotifyState_Ability::GetNotifyName_Implementation() const
{
	return TEXT("Activate Ability");
}
