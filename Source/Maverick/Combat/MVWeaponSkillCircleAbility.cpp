#include "Combat/MVWeaponSkillCircleAbility.h"

#include "Combat/MVWeaponSkillSupport.h"

void UMVWeaponSkillCircleAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	if (bAbilityActive)
	{
		return;
	}
	Super::StartAbility_Implementation(AbilityIndex);
	if (bAbilityActive)
	{
		if (Window)
		{
			Window->Close();
		}
		Window = NewObject<UMVWeaponSkillAreaWindow>(this);
		Window->Open(this, UMVWeaponSkillAreaWindow::EShape::Circle, OuterRadius, InnerRadius);
	}
}

void UMVWeaponSkillCircleAbility::EndAbility_Implementation()
{
	if (Window)
	{
		Window->Close();
		Window = nullptr;
	}
	Super::EndAbility_Implementation();
}
