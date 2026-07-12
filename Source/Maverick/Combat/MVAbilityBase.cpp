// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MVAbilityBase.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Components/MVCombatComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVAbilityBase, Log, All);

namespace
{
void MVAbilityLogHitLaunchTrace(
	const UObject* Source,
	const TCHAR* Stage,
	const FMVHitLaunchData& LaunchData)
{
	UE_LOG(
		LogMVAbilityBase,
		Log,
		TEXT("HitLaunchTrace Frame=%llu Stage=%s Source=%s Distance=%.2f Duration=%.3f VerticalSpeed=%.2f"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Source),
		LaunchData.LaunchDistance,
		LaunchData.LaunchDuration,
		LaunchData.LaunchVerticalSpeed);
}
}

void UMVAbilityBase::SetOwner(UActorComponent* Owner)
{
	if (Owner)
	{
		OwnerComponent = Owner;
	}
}

UActorComponent* UMVAbilityBase::GetOwner()
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
	PrepareAbilityExecution();
}

FMVHitLaunchData UMVAbilityBase::GetHitLaunchData_Implementation() const
{
	MVAbilityLogHitLaunchTrace(this, TEXT("AbilityDefault"), HitLaunchData);
	return HitLaunchData;
}

void UMVAbilityBase::ApplyHitLaunchDataToResolveRequest(FMVHitResolveRequest& Request) const
{
	Request.HitLaunchData = GetHitLaunchData();
	MVAbilityLogHitLaunchTrace(this, TEXT("AbilityApplyToRequest"), Request.HitLaunchData);
}

void UMVAbilityBase::PrepareAbilityExecution()
{
	bAbilityActive = false;
	bAbilityCostConsumed = false;
}

void UMVAbilityBase::StartAbility_Implementation(int32 AbilityIndex)
{
	if (bAbilityActive)
	{
		return;
	}

	if (!bAbilityCostConsumed && !TryConsumeAbilityCost())
	{
		return;
	}

	bAbilityCostConsumed = true;
	bAbilityActive = true;
}

void UMVAbilityBase::EndAbility_Implementation()
{
	if (!bAbilityActive)
	{
		return;
	}

	bAbilityActive = false;

	UMVCombatComponent* OwnerCombatComponent = Cast<UMVCombatComponent>(OwnerComponent);
	if (OwnerCombatComponent)
	{
		OwnerCombatComponent->HandleAbilityEnded(this);
	}
}

void UMVAbilityBase::ActiveHitStopToCharacters(AMVCharacterBase* Owner, AMVCharacterBase* Target, float Duration, float DilationAmount)
{
	// Even If One of the actor's are not valid, HitStop still can activate to the valid one.
	
	if (Owner)
	{
		Owner->ActiveHitstop(Duration, DilationAmount);
	}

	if (Target)
	{
		Target->ActiveHitstop(Duration, DilationAmount);
	}

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

	if ((StaminaCost > 0.0f && !StatComponent->HasAnyStamina()) || !StatComponent->HasMP(MPCost))
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

	const bool bConsumedStamina = StatComponent->ConsumeStaminaAllowPartial(StaminaCost);
	const bool bConsumedMP = StatComponent->ConsumeMP(MPCost);
	return bConsumedStamina && bConsumedMP;
}

