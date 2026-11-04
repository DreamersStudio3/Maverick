#include "System/Progression/MVProgressionStatBinding.h"

#include "Components/MVStatComponent.h"
#include "Tags/MVGameplayTags.h"

TArray<FGameplayTag> FMVProgressionStatBinding::GetSupportedStatIds()
{
	return {
		MVGameplayTags::Stat_MaxHP,
		MVGameplayTags::Stat_MaxStamina,
		MVGameplayTags::Stat_MaxMP
	};
}

bool FMVProgressionStatBinding::IsSupportedStat(const FGameplayTag& StatId)
{
	return StatId == MVGameplayTags::Stat_MaxHP
		|| StatId == MVGameplayTags::Stat_MaxStamina
		|| StatId == MVGameplayTags::Stat_MaxMP;
}

bool FMVProgressionStatBinding::TryGetBaseValue(
	const UMVStatComponent& StatComponent,
	const FGameplayTag& StatId,
	float& OutValue)
{
	if (StatId == MVGameplayTags::Stat_MaxHP)
	{
		OutValue = StatComponent.GetBaseMaxHP();
		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxStamina)
	{
		OutValue = StatComponent.GetBaseMaxStamina();
		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxMP)
	{
		OutValue = StatComponent.GetBaseMaxMP();
		return true;
	}

	return false;
}

bool FMVProgressionStatBinding::TryGetEffectiveValue(
	const UMVStatComponent& StatComponent,
	const FGameplayTag& StatId,
	float& OutValue)
{
	if (StatId == MVGameplayTags::Stat_MaxHP)
	{
		OutValue = StatComponent.MaxHP;
		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxStamina)
	{
		OutValue = StatComponent.MaxStamina;
		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxMP)
	{
		OutValue = StatComponent.MaxMP;
		return true;
	}

	return false;
}

bool FMVProgressionStatBinding::TryApplyBonus(
	UMVStatComponent& StatComponent,
	const FGameplayTag& StatId,
	const float Bonus)
{
	float CheckedValue = 0.0f;
	if (!TryGetPreviewEffectiveValue(StatComponent, StatId, Bonus, CheckedValue))
	{
		return false;
	}

	if (StatId == MVGameplayTags::Stat_MaxHP)
	{
		StatComponent.SetMaxHP(StatComponent.GetBaseMaxHP() + Bonus);
		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxStamina)
	{
		StatComponent.SetMaxStamina(StatComponent.GetBaseMaxStamina() + Bonus);
		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxMP)
	{
		StatComponent.SetMaxMP(StatComponent.GetBaseMaxMP() + Bonus);
		return true;
	}

	return false;
}

bool FMVProgressionStatBinding::TryGetPreviewEffectiveValue(
	const UMVStatComponent& StatComponent,
	const FGameplayTag& StatId,
	float PreviewBonus,
	float& OutValue)
{
	OutValue = 0.0f;
	if (!FMath::IsFinite(PreviewBonus))
	{
		return false;
	}

	if (StatId == MVGameplayTags::Stat_MaxHP)
	{
		return StatComponent.TryCalculateMaxHPWithProgressionBonus(PreviewBonus, OutValue);
	}

	float BaseValue = 0.0f;
	if (!TryGetBaseValue(StatComponent, StatId, BaseValue)
		|| !FMath::IsFinite(BaseValue + PreviewBonus))
	{
		return false;
	}

	OutValue = FMath::Max(0.0f, BaseValue + PreviewBonus);
	return true;
}

bool FMVProgressionStatBinding::TryGetDisplayInfo(
	const FGameplayTag& StatId,
	FText& OutDisplayName,
	bool& bOutLowerIsBetter)
{
	OutDisplayName = FText::GetEmpty();
	bOutLowerIsBetter = false;

	if (StatId == MVGameplayTags::Stat_MaxHP)
	{
		OutDisplayName = NSLOCTEXT(
			"MaverickProgression",
			"MaxHPDisplayName",
			"Max HP");

		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxStamina)
	{
		OutDisplayName = NSLOCTEXT(
			"MaverickProgression",
			"MaxStaminaDisplayName",
			"Max Stamina");

		return true;
	}

	if (StatId == MVGameplayTags::Stat_MaxMP)
	{
		OutDisplayName = NSLOCTEXT(
			"MaverickProgression",
			"MaxMPDisplayName",
			"Max MP");

		return true;
	}

	return false;
}
