#include "Animation/NotifyStates/MVAnimNotifyState_Ability.h"

#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interface/MVAbilityInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVAnimNotifyStateAbility, Log, All);

namespace
{
UMVCombatComponent* FindAbilityNotifyCombatComponent(const USkeletalMeshComponent* MeshComp)
{
	const AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	return OwnerActor ? OwnerActor->FindComponentByClass<UMVCombatComponent>() : nullptr;
}

bool MVAbilityNotifyCanRunAbilityNotifyState(
	const UMVCombatComponent* CombatComponent,
	const UMVAbilityBase* CurrentAbility,
	const TSubclassOf<UMVAbilityBase> AbilityClass,
	const TCHAR* PhaseName,
	const UAnimSequenceBase* Animation)
{
	if (!CombatComponent)
	{
		UE_LOG(
			LogMVAnimNotifyStateAbility,
			Warning,
			TEXT("MV Activate Ability %s skipped because the owner has no MVCombatComponent. Animation=%s."),
			PhaseName,
			*GetNameSafe(Animation));
		return false;
	}

	if (!CurrentAbility)
	{
		UE_LOG(
			LogMVAnimNotifyStateAbility,
			Warning,
			TEXT("MV Activate Ability %s skipped because CombatComponent has no CurrentAbilityInstance. Animation=%s."),
			PhaseName,
			*GetNameSafe(Animation));
		return false;
	}

	if (AbilityClass && !CurrentAbility->IsA(AbilityClass))
	{
		UE_LOG(
			LogMVAnimNotifyStateAbility,
			Warning,
			TEXT("MV Activate Ability %s skipped because AbilityClass does not match. Expected=%s Current=%s Animation=%s."),
			PhaseName,
			*GetNameSafe(AbilityClass.Get()),
			*GetNameSafe(CurrentAbility->GetClass()),
			*GetNameSafe(Animation));
		return false;
	}

	return true;
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
	if (!MVAbilityNotifyCanRunAbilityNotifyState(CombatComponent, CurrentAbility, AbilityClass, TEXT("Begin"), Animation))
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
	if (!MVAbilityNotifyCanRunAbilityNotifyState(CombatComponent, CurrentAbility, AbilityClass, TEXT("End"), Animation))
	{
		return;
	}

	IMVAbilityInterface::Execute_EndAbility(CurrentAbility);
	CurrentAbility->EnsureAbilityEnded();
}

FString UMVAnimNotifyState_Ability::GetNotifyName_Implementation() const
{
	return TEXT("Activate Ability");
}
