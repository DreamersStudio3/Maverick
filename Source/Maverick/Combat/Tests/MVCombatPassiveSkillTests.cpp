#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Combat/MVAbilityBase.h"
#include "Components/MVCombatComponent.h"
#include "Engine/DataTable.h"
#include "Tables/MVSkillDataTableColumn.h"
#include "Tables/MVTableTypes.h"

namespace
{
FDataTableRowHandle MVCombatPassiveTestAddRow(
	UDataTable& DataTable,
	const FName RowName,
	const float HpCost = 0.0f)
{
	FMVSkillDataTableColumn Row;
	Row.AbilityReference = UMVAbilityBase::StaticClass();
	Row.HpCost = HpCost;
	DataTable.AddRow(RowName, Row);

	FDataTableRowHandle RowHandle;
	RowHandle.DataTable = &DataTable;
	RowHandle.RowName = RowName;
	return RowHandle;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVCombatPassiveSkillLifecycleTest,
	"Maverick.Combat.PassiveSkill.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVCombatPassiveSkillLifecycleTest::RunTest(const FString& Parameters)
{
	UDataTable* PassiveSkillTable = NewObject<UDataTable>();
	PassiveSkillTable->RowStruct = FMVSkillDataTableColumn::StaticStruct();
	const FDataTableRowHandle PassiveRow = MVCombatPassiveTestAddRow(*PassiveSkillTable, TEXT("PassiveA"));

	UMVCombatComponent* CombatComponent = NewObject<UMVCombatComponent>();
	TestTrue(TEXT("Passive applies"), CombatComponent->ApplyPassiveSkill(PassiveRow));
	UMVAbilityBase* FirstInstance = CombatComponent->GetActivePassiveSkill();
	TestNotNull(TEXT("Active instance is stored"), FirstInstance);
	TestTrue(TEXT("Active instance started"), FirstInstance && FirstInstance->bAbilityActive);
	TestTrue(TEXT("Active row is stored"), CombatComponent->GetActivePassiveSkillRowHandle() == PassiveRow);

	TestTrue(TEXT("Applying the same passive is idempotent"), CombatComponent->ApplyPassiveSkill(PassiveRow));
	TestTrue(TEXT("Same passive keeps its instance"), CombatComponent->GetActivePassiveSkill() == FirstInstance);

	CombatComponent->ClearPassiveSkill();
	TestNull(TEXT("Clear removes the active instance"), CombatComponent->GetActivePassiveSkill());
	TestTrue(TEXT("Clear resets the active row"), CombatComponent->GetActivePassiveSkillRowHandle().RowName.IsNone());
	TestFalse(TEXT("Clear deactivates the previous instance"), FirstInstance && FirstInstance->bAbilityActive);

	CombatComponent->ClearPassiveSkill();
	TestNull(TEXT("Repeated clear remains safe"), CombatComponent->GetActivePassiveSkill());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVCombatPassiveSkillFailedReplacementTest,
	"Maverick.Combat.PassiveSkill.FailedReplacementLeavesVacancy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVCombatPassiveSkillFailedReplacementTest::RunTest(const FString& Parameters)
{
	UDataTable* PassiveSkillTable = NewObject<UDataTable>();
	PassiveSkillTable->RowStruct = FMVSkillDataTableColumn::StaticStruct();
	const FDataTableRowHandle ValidPassiveRow = MVCombatPassiveTestAddRow(*PassiveSkillTable, TEXT("ValidPassive"));
	const FDataTableRowHandle CostPassiveRow = MVCombatPassiveTestAddRow(*PassiveSkillTable, TEXT("CostPassive"), 1.0f);
	FMVSkillDataTableColumn MissingAbilityRowData;
	PassiveSkillTable->AddRow(TEXT("MissingAbilityPassive"), MissingAbilityRowData);
	FDataTableRowHandle MissingAbilityRow;
	MissingAbilityRow.DataTable = PassiveSkillTable;
	MissingAbilityRow.RowName = TEXT("MissingAbilityPassive");

	UDataTable* WrongRowTable = NewObject<UDataTable>();
	WrongRowTable->RowStruct = FMVTableRowBase::StaticStruct();
	WrongRowTable->AddRow(TEXT("WrongRow"), FMVTableRowBase());
	FDataTableRowHandle WrongRow;
	WrongRow.DataTable = WrongRowTable;
	WrongRow.RowName = TEXT("WrongRow");

	UMVCombatComponent* CombatComponent = NewObject<UMVCombatComponent>();
	TestTrue(TEXT("Initial passive applies"), CombatComponent->ApplyPassiveSkill(ValidPassiveRow));
	UMVAbilityBase* PreviousInstance = CombatComponent->GetActivePassiveSkill();

	TestFalse(TEXT("Passive with a cost is rejected"), CombatComponent->ApplyPassiveSkill(CostPassiveRow));
	TestNull(TEXT("Failed replacement leaves the passive slot vacant"), CombatComponent->GetActivePassiveSkill());
	TestTrue(
		TEXT("Failed replacement clears the active row"),
		CombatComponent->GetActivePassiveSkillRowHandle().RowName.IsNone());
	TestFalse(TEXT("Failed replacement deactivates the previous passive"), PreviousInstance && PreviousInstance->bAbilityActive);

	TestTrue(TEXT("A valid passive can be applied again after vacancy"), CombatComponent->ApplyPassiveSkill(ValidPassiveRow));
	TestFalse(TEXT("Passive without an AbilityReference is rejected"), CombatComponent->ApplyPassiveSkill(MissingAbilityRow));
	TestNull(TEXT("Missing AbilityReference leaves the slot vacant"), CombatComponent->GetActivePassiveSkill());

	TestTrue(TEXT("A valid passive can be reapplied before row type validation"), CombatComponent->ApplyPassiveSkill(ValidPassiveRow));
	TestFalse(TEXT("A row with the wrong struct is rejected"), CombatComponent->ApplyPassiveSkill(WrongRow));
	TestNull(TEXT("Wrong row type leaves the slot vacant"), CombatComponent->GetActivePassiveSkill());
	return true;
}

#endif
