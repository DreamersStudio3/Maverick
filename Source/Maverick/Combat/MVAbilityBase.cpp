// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MVAbilityBase.h"

#include "Components/MVCombatComponent.h"
#include "GameFramework/Character.h"

void UMVAbilityBase::SetOwner(UMVCombatComponent* Owner)
{
	if (Owner)
	{
		OwnerComponent = Owner;
	}
}

UMVCombatComponent* UMVAbilityBase::GetOwner()
{
	return OwnerComponent;
}

ACharacter* UMVAbilityBase::GetComponentOwner()
{
	if (OwnerComponent)
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerComponent->GetOwner());
		
		return OwnerCharacter;
	}

	return nullptr;
}

void UMVAbilityBase::InitAbility(FMVSkillDataTableColumn Data)
{
	AbilityData = Data;
}

void UMVAbilityBase::StartAbility_Implementation()
{
	
}

void UMVAbilityBase::EndAbility_Implementation()
{
}

