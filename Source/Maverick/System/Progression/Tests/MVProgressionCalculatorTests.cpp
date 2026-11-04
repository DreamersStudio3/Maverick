#include "Misc/AutomationTest.h"

#include "System/MVWorldStateTypes.h"
#include "System/Progression/MVProgressionCalculatorLibrary.h"
#include "System/Progression/MVProgressionDefinition.h"
#include "Tags/MVGameplayTags.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProgressionDefaultAllocationTest,
	"Maverick.Progression.DefaultAllocation",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionDefaultAllocationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UMVProgressionDefinition* Definition = NewObject<UMVProgressionDefinition>();

	if (!TestNotNull(TEXT("Progression definition"), Definition))
	{
		return false;
	}

	TestEqual(TEXT("Default entry count"), Definition->Entries.Num(), 3);

	FMVPlayerProgressionSaveData SaveData;
	SaveData.Currency = 10000;

	TMap<FGameplayTag, int32> PendingRanks;
	PendingRanks.Add(MVGameplayTags::Progression_Attribute_HP, 1);
	PendingRanks.Add(MVGameplayTags::Progression_Attribute_Stamina, 1);
	PendingRanks.Add(MVGameplayTags::Progression_Attribute_MP, 1);

	const FMVProgressionEvaluation Evaluation =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			PendingRanks);

	TestEqual(
		TEXT("Evaluation result"),
		Evaluation.Result,
		EMVProgressionEvaluationResult::Success);

	TestEqual(
		TEXT("Current level"),
		Evaluation.CurrentLevel,
		1);

	TestEqual(
		TEXT("Preview level"),
		Evaluation.PreviewLevel,
		4);

	TestEqual(
		TEXT("Total cost"),
		Evaluation.TotalCost,
		static_cast<int64>(450));

	TestEqual(
		TEXT("Remaining currency"),
		Evaluation.RemainingCurrency,
		static_cast<int64>(9550));

	TestEqual(
		TEXT("Max HP bonus"),
		Evaluation.PreviewStatBonuses.FindRef(
			MVGameplayTags::Stat_MaxHP),
		20.0f);

	TestEqual(
		TEXT("Max stamina bonus"),
		Evaluation.PreviewStatBonuses.FindRef(
			MVGameplayTags::Stat_MaxStamina),
		10.0f);

	TestEqual(
		TEXT("Max MP bonus"),
		Evaluation.PreviewStatBonuses.FindRef(
			MVGameplayTags::Stat_MaxMP),
		10.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProgressionValidationTest,
	"Maverick.Progression.Validation",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UMVProgressionDefinition* Definition = NewObject<UMVProgressionDefinition>();

	if (!TestNotNull(TEXT("Progression definition"), Definition))
	{
		return false;
	}

	FMVPlayerProgressionSaveData SaveData;
	SaveData.Currency = 100;

	TMap<FGameplayTag, int32> PendingRanks;
	PendingRanks.Add(MVGameplayTags::Progression_Attribute_HP, 2);

	FMVProgressionEvaluation Evaluation =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			PendingRanks);

	TestEqual(
		TEXT("Insufficient currency"),
		Evaluation.Result,
		EMVProgressionEvaluationResult::InsufficientCurrency);

	SaveData.Currency = MAX_int64;

	PendingRanks[MVGameplayTags::Progression_Attribute_HP] = 100;

	Evaluation =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			PendingRanks);

	TestEqual(
		TEXT("Per-entry rank limit"),
		Evaluation.Result,
		EMVProgressionEvaluationResult::RankLimitExceeded);

	PendingRanks.Reset();
	PendingRanks.Add(FGameplayTag(), 1);

	Evaluation =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			PendingRanks);

	TestEqual(
		TEXT("Unknown progression"),
		Evaluation.Result,
		EMVProgressionEvaluationResult::UnknownProgression);

	PendingRanks.Reset();
	PendingRanks.Add(MVGameplayTags::Progression_Attribute_HP, -1);

	Evaluation =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			PendingRanks);

	TestEqual(
		TEXT("Negative allocation"),
		Evaluation.Result,
		EMVProgressionEvaluationResult::NegativeAllocation);

	PendingRanks[MVGameplayTags::Progression_Attribute_HP] = 2;

	Definition->BaseLevelCost = MAX_int64;
	Definition->CostPerLevel = 1;

	Evaluation =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			PendingRanks);

	TestEqual(
		TEXT("Cost overflow"),
		Evaluation.Result,
		EMVProgressionEvaluationResult::CostOverflow);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProgressionAllocationOrderTest,
	"Maverick.Progression.AllocationOrder",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionAllocationOrderTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UMVProgressionDefinition* Definition = NewObject<UMVProgressionDefinition>();

	if (!TestNotNull(TEXT("Progression definition"), Definition))
	{
		return false;
	}

	FMVPlayerProgressionSaveData SaveData;
	SaveData.Currency = 10000;

	TMap<FGameplayTag, int32> FirstOrder;
	FirstOrder.Add(MVGameplayTags::Progression_Attribute_HP, 2);
	FirstOrder.Add(MVGameplayTags::Progression_Attribute_MP, 1);

	TMap<FGameplayTag, int32> SecondOrder;
	SecondOrder.Add(MVGameplayTags::Progression_Attribute_MP, 1);
	SecondOrder.Add(MVGameplayTags::Progression_Attribute_HP, 2);

	const FMVProgressionEvaluation First =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			FirstOrder);

	const FMVProgressionEvaluation Second =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			Definition,
			SaveData,
			SecondOrder);

	TestEqual(
		TEXT("First evaluation succeeds"),
		First.Result,
		EMVProgressionEvaluationResult::Success);

	TestEqual(
		TEXT("Second evaluation succeeds"),
		Second.Result,
		EMVProgressionEvaluationResult::Success);

	TestEqual(
		TEXT("Order-independent cost"),
		First.TotalCost,
		Second.TotalCost);

	TestEqual(
		TEXT("Order-independent level"),
		First.PreviewLevel,
		Second.PreviewLevel);

	TestEqual(
		TEXT("Order-independent HP bonus"),
		First.PreviewStatBonuses.FindRef(
			MVGameplayTags::Stat_MaxHP),
		Second.PreviewStatBonuses.FindRef(
			MVGameplayTags::Stat_MaxHP));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVProgressionSaveDefaultsTest,
	"Maverick.Progression.SaveDefaults",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::EngineFilter)

bool FMVProgressionSaveDefaultsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const FMVWorldSaveData WorldSaveData;

	TestEqual(
		TEXT("Default progression data version"),
		WorldSaveData.PlayerProgression.DataVersion,
		1);

	TestEqual(
		TEXT("Default base level"),
		WorldSaveData.PlayerProgression.BaseLevel,
		1);

	TestEqual(
		TEXT("Default currency"),
		WorldSaveData.PlayerProgression.Currency,
		static_cast<int64>(0));

	TestEqual(
		TEXT("Default revision"),
		WorldSaveData.PlayerProgression.Revision,
		0);

	TestEqual(
		TEXT("Default invested rank count"),
		WorldSaveData.PlayerProgression.InvestedRanks.Num(),
		0);

	return true;
}

#endif