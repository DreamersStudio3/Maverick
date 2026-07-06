// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MVAbilityBase.h"

#include "Components/MVCombatComponent.h"
#include "Character/MVCharacterBase.h"

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

AMVCharacterBase* UMVAbilityBase::GetOwnerCharacter()
{
	if (OwnerComponent)
	{
		AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(OwnerComponent->GetOwner());
		if (OwnerCharacter)
		{
			return OwnerCharacter;
		}
		else
		{
			return nullptr;
		}
		
	}

	return nullptr;
}

void UMVAbilityBase::InitAbility(const FMVSkillDataTableColumn& Data)
{
	AbilityData = Data;
}

void UMVAbilityBase::StartAbility_Implementation()
{
	if (bAbilityActive)
	{
		return;
	}
	bAbilityActive = true;
}

void UMVAbilityBase::EndAbility_Implementation()
{
	if (bAbilityActive)
	{
		return;
	}
	bAbilityActive = false;
}

