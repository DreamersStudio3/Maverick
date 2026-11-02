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
	FMVProgressionMaxHPModifierTest,
	"Maverick.Progression.StatBinding.MaxHPModifier",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionMaxHPModifierTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UMVStatComponent* StatComponent = NewObject<UMVStatComponent>();
	if (!TestNotNull(TEXT("Stat component"), StatComponent))
	{
		return false;
	}

	StatComponent->SetCurrentHP(60.0f);
	const FGuid FirstHandle = StatComponent->AddMaxHPModifier(0.20f);
	TestTrue(TEXT("Max HP modifier registered"), FirstHandle.IsValid());
	TestTrue(TEXT("Base Max HP increases by 20 percent"),
		FMath::IsNearlyEqual(StatComponent->MaxHP, 120.0f, 0.001f));
	TestEqual(TEXT("Increasing maximum does not heal"), StatComponent->CurrentHP, 60.0f);
	TestEqual(TEXT("Modifier changes calculation revision"),
		StatComponent->GetStatCalculationRevision(), 1);

	float PreviewMaxHP = 0.0f;
	TestTrue(TEXT("Preview with growth bonus succeeds"),
		FMVProgressionStatBinding::TryGetPreviewEffectiveValue(
			*StatComponent, MVGameplayTags::Stat_MaxHP, 40.0f, PreviewMaxHP));
	TestTrue(TEXT("Preview includes growth and modifier"),
		FMath::IsNearlyEqual(PreviewMaxHP, 168.0f, 0.001f));

	TMap<FGameplayTag, float> Bonuses;
	Bonuses.Add(MVGameplayTags::Stat_MaxHP, 40.0f);
	TestTrue(TEXT("Growth replacement succeeds while modified"),
		StatComponent->ReplaceProgressionStatBonuses(Bonuses));
	TestTrue(TEXT("Applied Max HP matches preview"),
		FMath::IsNearlyEqual(StatComponent->MaxHP, PreviewMaxHP, 0.001f));
	TestEqual(TEXT("Growth and modifier do not heal"), StatComponent->CurrentHP, 60.0f);
	TestEqual(TEXT("Growth changes calculation revision"),
		StatComponent->GetStatCalculationRevision(), 2);

	TestTrue(TEXT("Next growth preview succeeds"),
		FMVProgressionStatBinding::TryGetPreviewEffectiveValue(
			*StatComponent, MVGameplayTags::Stat_MaxHP, 60.0f, PreviewMaxHP));
	TestTrue(TEXT("Next growth preview includes modifier"),
		FMath::IsNearlyEqual(PreviewMaxHP, 192.0f, 0.001f));

	StatComponent->SetCurrentHP(160.0f);
	const FGuid SecondHandle = StatComponent->AddMaxHPModifier(0.10f);
	TestTrue(TEXT("Second Max HP modifier registered"), SecondHandle.IsValid());
	TestTrue(TEXT("Two modifier ratios add"),
		FMath::IsNearlyEqual(StatComponent->MaxHP, 182.0f, 0.001f));
	TestEqual(TEXT("Adding second modifier keeps current HP"), StatComponent->CurrentHP, 160.0f);
	TestTrue(TEXT("First modifier removes independently"),
		StatComponent->RemoveMaxHPModifier(FirstHandle));
	TestTrue(TEXT("Remaining modifier stays on growth-adjusted base"),
		FMath::IsNearlyEqual(StatComponent->MaxHP, 154.0f, 0.001f));
	TestTrue(TEXT("Maximum reduction only clamps excess current HP"),
		FMath::IsNearlyEqual(StatComponent->CurrentHP, 154.0f, 0.001f));

	Bonuses.Add(MVGameplayTags::Stat_MaxHP, 60.0f);
	TestTrue(TEXT("Later growth replacement succeeds"),
		StatComponent->ReplaceProgressionStatBonuses(Bonuses));
	TestTrue(TEXT("Later growth retains remaining modifier"),
		FMath::IsNearlyEqual(StatComponent->MaxHP, 176.0f, 0.001f));
	TestTrue(TEXT("Final modifier removes independently"),
		StatComponent->RemoveMaxHPModifier(SecondHandle));
	TestTrue(TEXT("Latest growth remains after all modifiers leave"),
		FMath::IsNearlyEqual(StatComponent->MaxHP, 160.0f, 0.001f));
	TestTrue(TEXT("Removing final modifier does not heal"),
		FMath::IsNearlyEqual(StatComponent->CurrentHP, 154.0f, 0.001f));

	StatComponent->SetCurrentHP(0.0f);
	const FGuid DeadHandle = StatComponent->AddMaxHPModifier(0.20f);
	TestTrue(TEXT("Modifier can recalculate while dead"), DeadHandle.IsValid());
	TestEqual(TEXT("Dead character stays at zero HP"), StatComponent->CurrentHP, 0.0f);
	TestTrue(TEXT("Dead character modifier removes"),
		StatComponent->RemoveMaxHPModifier(DeadHandle));
	TestEqual(TEXT("Removing modifier does not revive"), StatComponent->CurrentHP, 0.0f);

	UMVStatComponent* SwapStats = NewObject<UMVStatComponent>();
	if (!TestNotNull(TEXT("Swap stat component"), SwapStats))
	{
		return false;
	}
	const FGuid OriginalHandle = SwapStats->AddMaxHPModifier(0.20f);
	TestTrue(TEXT("Swap original modifier registered"), OriginalHandle.IsValid());
	SwapStats->SetCurrentHP(120.0f);
	SwapStats->BeginMaxHPModifierBatch();
	TestTrue(TEXT("Swap original modifier removed"),
		SwapStats->RemoveMaxHPModifier(OriginalHandle));
	TestEqual(TEXT("Swap does not clamp HP between passive instances"), SwapStats->CurrentHP, 120.0f);
	const FGuid ReplacementHandle = SwapStats->AddMaxHPModifier(0.20f);
	TestTrue(TEXT("Swap replacement modifier registered"), ReplacementHandle.IsValid());
	SwapStats->EndMaxHPModifierBatch();
	TestEqual(TEXT("Same passive swap keeps maximum HP"), SwapStats->MaxHP, 120.0f);
	TestEqual(TEXT("Same passive swap keeps current HP"), SwapStats->CurrentHP, 120.0f);
	TestEqual(TEXT("Swap changes calculation revision once"),
		SwapStats->GetStatCalculationRevision(), 2);

	SwapStats->BeginMaxHPModifierBatch();
	TestTrue(TEXT("Final passive modifier removed"),
		SwapStats->RemoveMaxHPModifier(ReplacementHandle));
	SwapStats->EndMaxHPModifierBatch();
	TestEqual(TEXT("Final passive removal restores maximum HP"), SwapStats->MaxHP, 100.0f);
	TestEqual(TEXT("Final passive removal clamps excess current HP"), SwapStats->CurrentHP, 100.0f);

	return !HasAnyErrors();
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
