// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/MVAbilityBase.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatusEffectComponent.h"
#include "MVAbilityBase.h"

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

void UMVAbilityBase::ApplyOnHitStatusEffect(const FMVResolvedHitData& HitData)
{
	if (AbilityData.OnHitStatusEffects.IsEmpty())
	{
		return;
	}

	AMVCharacterBase* SourceCharacter = GetOwnerCharacter();
	AMVCharacterBase* EventTargetCharacter = HitData.Victim.Get();

	if (!IsValid(SourceCharacter) || HitData.Attacker.Get() != SourceCharacter || !IsValid(EventTargetCharacter))
	{
		return;
	}

	for (const FMVStatusEffectApplication& Application : AbilityData.OnHitStatusEffects)
	{
		if (!Application.IsValid())
		{
			continue;
		}

		AMVCharacterBase* RecipientCharacter = nullptr;

		switch (Application.ApplicationTarget)
		{
		case EMVStatusEffectApplicationTarget::EventTarget:
			RecipientCharacter = EventTargetCharacter;
			break;

		case EMVStatusEffectApplicationTarget::SourceActor:
			RecipientCharacter = SourceCharacter;
			break;

		default:
			continue;
		}

		if (!IsValid(RecipientCharacter))
		{
			continue;
		}

		UMVStatusEffectComponent* StatusEffectComponent = RecipientCharacter->StatusEffectComponent.Get();

		if (!IsValid(StatusEffectComponent))
		{
			continue;
		}

		FMVStatusEffectSpec Spec;
		Spec.Definition = Application.Definition;
		Spec.SourceActor = SourceCharacter;
		Spec.StackDelta = Application.StackDelta;

		StatusEffectComponent->ApplyStatusEffect(Spec);
	}
}

void UMVAbilityBase::PrepareAbilityExecution()
{
	bAbilityActive = false;
	bAbilityCostConsumed = false;
	SourceSkillIndex = INDEX_NONE;
	SourceWeaponItemTag = FGameplayTag();
	ConsumedMPThisExecution = 0.0f;
}

void UMVAbilityBase::StartAbility_Implementation(int32 AbilityIndex)
{
	if (bAbilityActive)
	{
		return;
	}

	const bool bFirstCostPayment = !bAbilityCostConsumed;
	if (bFirstCostPayment && !TryConsumeAbilityCost())
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

void UMVAbilityBase::ActiveCameraShake(AMVCharacterBase* Owner, TSubclassOf<UCameraShakeBase> Shake, float Scale)
{
	APlayerController* PlayerController = Cast<APlayerController>(Owner->GetController());

	if (!PlayerController)
	{
		return;
	}

	PlayerController->ClientStartCameraShake(Shake, Scale, ECameraShakePlaySpace::World);
}

void UMVAbilityBase::TryVamp(float FinalDamage)
{
	AMVCharacterBase* OwnerCharacter = GetOwnerCharacter();

	if (OwnerCharacter == nullptr)
	{
		return;
	}

	UMVStatComponent* StatComponent = OwnerCharacter->FindComponentByClass<UMVStatComponent>();
	if (StatComponent == nullptr)
	{
		return;
	}

	float VampAmount = FinalDamage * StatComponent->NormalVamp;
	if (VampAmount <= 0)
	{
		return;
	}
	StatComponent->RecoverHP(VampAmount);
}

bool UMVAbilityBase::TryConsumeAbilityCost()
{
	const float HPCost = FMath::Max(0.0f, AbilityData.HpCost);
	const float StaminaCost = FMath::Max(0.0f, AbilityData.StaminaCost);
	const float MPCost = FMath::Max(0.0f, AbilityData.MpCost);
	if (HPCost <= 0.0f && StaminaCost <= 0.0f && MPCost <= 0.0f)
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

	if (!StatComponent->CanConsumeHP(HPCost)
		|| (StaminaCost > 0.0f && !StatComponent->HasAnyStamina())
		|| !StatComponent->HasMP(MPCost))
	{
		UE_LOG(
			LogMVAbilityBase,
			Verbose,
			TEXT("Not enough resources to start ability. Owner=%s, HPCost=%.2f, StaminaCost=%.2f, MPCost=%.2f."),
			*GetNameSafe(OwnerCharacter),
			HPCost,
			StaminaCost,
			MPCost);
		return false;
	}

	const bool bConsumedHP = StatComponent->ConsumeHP(HPCost);
	const bool bConsumedStamina = StatComponent->ConsumeStaminaAllowPartial(StaminaCost);
	const float MPBeforePayment = StatComponent->CurrentMP;
	const bool bConsumedMP = StatComponent->ConsumeMP(MPCost);

	if (!bConsumedHP || !bConsumedStamina || !bConsumedMP)
	{
		return false;
	}

	ConsumedMPThisExecution =
		FMath::Max(0.0f, MPBeforePayment - StatComponent->CurrentMP);
	return true;
}

