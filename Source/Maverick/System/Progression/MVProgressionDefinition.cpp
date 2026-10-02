#include "System/Progression/MVProgressionDefinition.h"

#include "Tags/MVGameplayTags.h"

namespace
{
	FMVProgressionStatEffect MakeProgressionLinearEffect(
		const FGameplayTag StatId,
		const int32 MaxRank,
		const float BonusPerRank)
	{
		FMVProgressionStatEffect Effect;
		Effect.StatId = StatId;

		FRichCurve* Curve = Effect.BonusByRank.GetRichCurve();

		const FKeyHandle StartKey = Curve->AddKey(0.0f, 0.0f);

		const FKeyHandle EndKey = Curve->AddKey(
			static_cast<float>(MaxRank),
			BonusPerRank * static_cast<float>(MaxRank));

		Curve->SetKeyInterpMode(StartKey, RCIM_Linear);
		Curve->SetKeyInterpMode(EndKey, RCIM_Linear);

		return Effect;
	}

	FMVProgressionEntryDefinition MakeProgressionEntry(
		const FGameplayTag ProgressionId,
		const FGameplayTag StatId,
		const FText& DisplayName,
		const FText& Description,
		const int32 SortOrder,
		const float BonusPerRank)
	{
		FMVProgressionEntryDefinition Entry;
		Entry.ProgressionId = ProgressionId;
		Entry.DisplayName = DisplayName;
		Entry.Description = Description;
		Entry.MaxRank = 99;
		Entry.SortOrder = SortOrder;

		Entry.Effects.Add(MakeProgressionLinearEffect(StatId, Entry.MaxRank, BonusPerRank));

		return Entry;
	}
}

UMVProgressionDefinition::UMVProgressionDefinition()
{
	Entries.Add(
		MakeProgressionEntry(
			MVGameplayTags::Progression_Attribute_HP,
			MVGameplayTags::Stat_MaxHP,
			NSLOCTEXT(
				"MaverickProgression",
				"HPName",
				"HP"),
			NSLOCTEXT(
				"MaverickProgression",
				"HPDescription",
				"Increases maximum HP."),
			0,
			20.0f));

	Entries.Add(
		MakeProgressionEntry(
			MVGameplayTags::Progression_Attribute_Stamina,
			MVGameplayTags::Stat_MaxStamina,
			NSLOCTEXT(
				"MaverickProgression",
				"StaminaName",
				"Stamina"),
			NSLOCTEXT(
				"MaverickProgression",
				"StaminaDescription",
				"Increases maximum stamina."),
			1,
			10.0f));

	Entries.Add(
		MakeProgressionEntry(
			MVGameplayTags::Progression_Attribute_MP,
			MVGameplayTags::Stat_MaxMP,
			NSLOCTEXT(
				"MaverickProgression",
				"MPName",
				"MP"),
			NSLOCTEXT(
				"MaverickProgression",
				"MPDescription",
				"Increases maximum MP."),
			2,
			10.0f));
}

const FMVProgressionEntryDefinition*
UMVProgressionDefinition::FindEntry(
	const FGameplayTag ProgressionId) const
{
	return Entries.FindByPredicate(
		[ProgressionId](
			const FMVProgressionEntryDefinition& Entry)
		{
			return Entry.ProgressionId == ProgressionId;
		});
}