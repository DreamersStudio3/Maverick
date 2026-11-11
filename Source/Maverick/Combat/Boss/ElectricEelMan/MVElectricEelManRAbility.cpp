#include "Combat/Boss/ElectricEelMan/MVElectricEelManRAbility.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManSkillComponent.h"
#include "Character/MVCharacterBase.h"

bool UMVElectricEelManRAbility::CanActivateAbility()
{
	auto* Character = GetOwnerCharacter();
	auto* State = IsValid(Character) ? Character->FindComponentByClass<UMVElectricEelManSkillComponent>() : nullptr;
	return State && State->CanUseR();
}
void UMVElectricEelManRAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	if (!BeginSkill(AbilityIndex, true)) return;
	BurstCount = 0;
	LockOwnerMovement();
}
void UMVElectricEelManRAbility::TickSkill(float Elapsed)
{
	// 느린 프레임에서도 예정된 방출 횟수 보존
	while (bAbilityActive && BurstCount < 3 && Elapsed >= 3.0f + BurstCount)
	{
		++BurstCount;
		TSet<TWeakObjectPtr<AActor>> HitActors;
		HitSphere(GetOwnerCharacter()->GetActorLocation(), BurstRadius, 1.0f, EMVActionHitReactionType::Flinch, HitActors);
	}
	if (BurstCount >= 3) EndAbility_Implementation();
}
