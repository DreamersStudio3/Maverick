#include "Combat/Boss/ElectricEelMan/MVElectricEelManQAbility.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManSkillComponent.h"

void UMVElectricEelManQAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	const bool bFirstActivation = !bAbilityCostConsumed;
	Super::StartAbility_Implementation(AbilityIndex);
	if (bFirstActivation && bAbilityCostConsumed)
	{
		if (auto* State = UMVElectricEelManSkillComponent::GetOrCreate(GetOwnerCharacter()))
			State->RecordSkillUse();
	}
}

UMVElectricEelManQAbility::UMVElectricEelManQAbility()
{
	BoxSize = FVector(600.0f, 100.0f, 180.0f);
	BoxOffset = FVector(350.0f, 0.0f, 0.0f);
}

void UMVElectricEelManQAbility::GetBoxOffsets(TArray<FVector>& OutOffsets) const
{
	const FVector SideOffset(0.0f, FMath::Max(0.0f, TailSeparation) * 0.5f, 0.0f);
	OutOffsets.Reset();
	OutOffsets.Add(BoxOffset - SideOffset);
	OutOffsets.Add(BoxOffset + SideOffset);
}
