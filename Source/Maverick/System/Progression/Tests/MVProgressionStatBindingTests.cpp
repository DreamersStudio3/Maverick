#include "Misc/AutomationTest.h"

#include "Components/MVStatComponent.h"
#include "System/Progression/MVProgressionStatBinding.h"
#include "Tags/MVGameplayTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProgressionStatBindingRegistryTest,
	"Maverick.Progression.StatBinding.Registry",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionStatBindingRegistryTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;

	TestTrue(
		TEXT("Max HP is supported"),
		FMVProgressionStatBinding::IsSupportedStat(
			MVGameplayTags::Stat_MaxHP));

	TestTrue(
		TEXT("Max stamina is supported"),
		FMVProgressionStatBinding::IsSupportedStat(
			MVGameplayTags::Stat_MaxStamina));

	TestTrue(
		TEXT("Max MP is supported"),
		FMVProgressionStatBinding::IsSupportedStat(
			MVGameplayTags::Stat_MaxMP));

	TestFalse(
		TEXT("Progression ID is not a stat ID"),
		FMVProgressionStatBinding::IsSupportedStat(
			MVGameplayTags::Progression_Attribute_HP));

	const TArray<FGameplayTag> SupportedStatIds =
		FMVProgressionStatBinding::GetSupportedStatIds();

	TestEqual(
		TEXT("Supported stat count"),
		SupportedStatIds.Num(),
		3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProgressionStatBonusReplacementTest,
	"Maverick.Progression.StatBinding.BonusReplacement",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionStatBonusReplacementTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;

	UMVStatComponent* StatComponent =
		NewObject<UMVStatComponent>();

	if (!TestNotNull(TEXT("Stat component"), StatComponent))
	{
		return false;
	}

	StatComponent->SetCurrentHP(80.0f);
	StatComponent->SetCurrentStamina(60.0f);
	StatComponent->SetCurrentMP(40.0f);

	TMap<FGameplayTag, float> Bonuses;
	Bonuses.Add(MVGameplayTags::Stat_MaxHP, 20.0f);
	Bonuses.Add(MVGameplayTags::Stat_MaxStamina, 10.0f);
	Bonuses.Add(MVGameplayTags::Stat_MaxMP, 10.0f);

	TestTrue(
		TEXT("Initial bonus replacement succeeds"),
		StatComponent->ReplaceProgressionStatBonuses(Bonuses));

	TestEqual(TEXT("Max HP with bonus"), StatComponent->MaxHP, 120.0f);
	TestEqual(TEXT("Max stamina with bonus"), StatComponent->MaxStamina, 110.0f);
	TestEqual(TEXT("Max MP with bonus"), StatComponent->MaxMP, 110.0f);

	TestEqual(TEXT("Current HP remains unchanged"), StatComponent->CurrentHP, 80.0f);
	TestEqual(TEXT("Current stamina remains unchanged"), StatComponent->CurrentStamina, 60.0f);
	TestEqual(TEXT("Current MP remains unchanged"), StatComponent->CurrentMP, 40.0f);

	TestEqual(
		TEXT("Revision after first replacement"),
		StatComponent->GetStatCalculationRevision(),
		1);

	TestTrue(
		TEXT("Identical replacement succeeds"),
		StatComponent->ReplaceProgressionStatBonuses(Bonuses));

	TestEqual(
		TEXT("Identical replacement does not increase revision"),
		StatComponent->GetStatCalculationRevision(),
		1);

	StatComponent->SetCurrentHP(115.0f);

	const TMap<FGameplayTag, float> EmptyBonuses;

	TestTrue(
		TEXT("Empty replacement succeeds"),
		StatComponent->ReplaceProgressionStatBonuses(
			EmptyBonuses));

	TestEqual(TEXT("Max HP returns to base"), StatComponent->MaxHP, 100.0f);
	TestEqual(TEXT("Max stamina returns to base"), StatComponent->MaxStamina, 100.0f);
	TestEqual(TEXT("Max MP returns to base"), StatComponent->MaxMP, 100.0f);

	TestEqual(
		TEXT("Current HP clamps after maximum decreases"),
		StatComponent->CurrentHP,
		100.0f);

	TestEqual(
		TEXT("Current stamina remains below maximum"),
		StatComponent->CurrentStamina,
		60.0f);

	TestEqual(
		TEXT("Current MP remains below maximum"),
		StatComponent->CurrentMP,
		40.0f);

	TestEqual(
		TEXT("Revision after removing bonuses"),
		StatComponent->GetStatCalculationRevision(),
		2);

	float BaseValue = 0.0f;
	float Bonus = 0.0f;
	float EffectiveValue = 0.0f;

	TestTrue(
		TEXT("Max HP values can be queried"),
		StatComponent->TryGetProgressionStatValues(
			MVGameplayTags::Stat_MaxHP,
			BaseValue,
			Bonus,
			EffectiveValue));

	TestEqual(TEXT("Queried base Max HP"), BaseValue, 100.0f);
	TestEqual(TEXT("Queried Max HP bonus"), Bonus, 0.0f);
	TestEqual(TEXT("Queried effective Max HP"), EffectiveValue, 100.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProgressionStatBonusRejectionTest,
	"Maverick.Progression.StatBinding.AtomicRejection",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionStatBonusRejectionTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;

	UMVStatComponent* StatComponent =
		NewObject<UMVStatComponent>();

	if (!TestNotNull(TEXT("Stat component"), StatComponent))
	{
		return false;
	}

	TMap<FGameplayTag, float> InitialBonuses;
	InitialBonuses.Add(MVGameplayTags::Stat_MaxHP, 20.0f);

	TestTrue(
		TEXT("Initial valid replacement succeeds"),
		StatComponent->ReplaceProgressionStatBonuses(
			InitialBonuses));

	const int32 RevisionBeforeInvalidReplacement =
		StatComponent->GetStatCalculationRevision();

	TMap<FGameplayTag, float> InvalidBonuses;
	InvalidBonuses.Add(MVGameplayTags::Stat_MaxHP, 40.0f);
	InvalidBonuses.Add(
		MVGameplayTags::Progression_Attribute_HP,
		10.0f);

	TestFalse(
		TEXT("Unsupported stat rejects entire replacement"),
		StatComponent->ReplaceProgressionStatBonuses(
			InvalidBonuses));

	TestEqual(
		TEXT("Max HP remains unchanged after rejection"),
		StatComponent->MaxHP,
		120.0f);

	TestEqual(
		TEXT("Revision remains unchanged after rejection"),
		StatComponent->GetStatCalculationRevision(),
		RevisionBeforeInvalidReplacement);

	TMap<FGameplayTag, float> InvalidTagBonuses;
	InvalidTagBonuses.Add(FGameplayTag(), 10.0f);

	TestFalse(
		TEXT("Invalid tag rejects replacement"),
		StatComponent->ReplaceProgressionStatBonuses(
			InvalidTagBonuses));

	TestEqual(
		TEXT("Max HP remains unchanged after invalid tag"),
		StatComponent->MaxHP,
		120.0f);

	return true;
}

#endif