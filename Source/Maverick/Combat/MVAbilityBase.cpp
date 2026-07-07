// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MVAbilityBase.h"

#include "Components/MVCombatComponent.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVAbilityBase, Log, All);

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

	if (!TryConsumeAbilityCost())
	{
		return;
	}

	bAbilityActive = true;
}

void UMVAbilityBase::EndAbility_Implementation()
{
	if (!bAbilityActive)
	{
		return;
	}

	bAbilityActive = false;
}

bool UMVAbilityBase::TryConsumeAbilityCost()
{
	const float StaminaCost = FMath::Max(0.0f, AbilityData.StaminaCost);
	const float MPCost = FMath::Max(0.0f, AbilityData.MpCost);
	if (StaminaCost <= 0.0f && MPCost <= 0.0f)
	{
		return true;
	}

	AMVCharacterBase* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		UE_LOG(LogMVAbilityBase, Warning, TEXT("Cannot consume ability cost without an owner character."));
		return false;
	}

	UMVStatComponent* StatComponent = OwnerCharacter->FindComponentByClass<UMVStatComponent>();
	if (!StatComponent)
	{
		UE_LOG(
			LogMVAbilityBase,
			Warning,
			TEXT("Cannot consume ability cost because %s has no MVStatComponent."),
			*GetNameSafe(OwnerCharacter));
		return false;
	}

	if (!StatComponent->HasStamina(StaminaCost) || !StatComponent->HasMP(MPCost))
	{
		UE_LOG(
			LogMVAbilityBase,
			Verbose,
			TEXT("Not enough resources to start ability. Owner=%s, StaminaCost=%.2f, MPCost=%.2f."),
			*GetNameSafe(OwnerCharacter),
			StaminaCost,
			MPCost);
		return false;
	}

	const bool bConsumedStamina = StatComponent->ConsumeStamina(StaminaCost);
	const bool bConsumedMP = StatComponent->ConsumeMP(MPCost);
	return bConsumedStamina && bConsumedMP;
}

