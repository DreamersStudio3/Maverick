#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "DataAsset/MVWeaponSkillMap.h"
#include "Engine/DataTable.h"

namespace
{
FDataTableRowHandle MVWeaponSkillMapTestMakeRow(UDataTable& DataTable, const FName RowName)
{
	FDataTableRowHandle RowHandle;
	RowHandle.DataTable = &DataTable;
	RowHandle.RowName = RowName;
	return RowHandle;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVWeaponSkillMapBasicAttackRowsTest,
	"Maverick.Combat.WeaponSkillMap.BasicAttackRows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVWeaponSkillMapBasicAttackRowsTest::RunTest(const FString& Parameters)
{
	UDataTable* DataTable = NewObject<UDataTable>();
	UMVWeaponSkillMap* WeaponSkillMap = NewObject<UMVWeaponSkillMap>();

	WeaponSkillMap->BasicAttacks.LightAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("LightAttack"));
	WeaponSkillMap->BasicAttacks.DodgeLightAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("DodgeLightAttack"));
	WeaponSkillMap->BasicAttacks.SprintLightAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("SprintLightAttack"));
	WeaponSkillMap->BasicAttacks.HeavyAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("HeavyAttack"));
	WeaponSkillMap->BasicAttacks.DodgeHeavyAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("DodgeHeavyAttack"));
	WeaponSkillMap->BasicAttacks.SprintHeavyAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("SprintHeavyAttack"));
	WeaponSkillMap->BasicAttacks.ChargeAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("ChargeAttack"));

	const TPair<EMVCombatActionTypes, FDataTableRowHandle> ExpectedRows[] =
	{
		{EMVCombatActionTypes::LightAttack, WeaponSkillMap->BasicAttacks.LightAttack},
		{EMVCombatActionTypes::DodgeLightAttack, WeaponSkillMap->BasicAttacks.DodgeLightAttack},
		{EMVCombatActionTypes::SprintLightAttack, WeaponSkillMap->BasicAttacks.SprintLightAttack},
		{EMVCombatActionTypes::HeavyAttack, WeaponSkillMap->BasicAttacks.HeavyAttack},
		{EMVCombatActionTypes::DodgeHeavyAttack, WeaponSkillMap->BasicAttacks.DodgeHeavyAttack},
		{EMVCombatActionTypes::SprintHeavyAttack, WeaponSkillMap->BasicAttacks.SprintHeavyAttack},
		{EMVCombatActionTypes::ChargeAttack, WeaponSkillMap->BasicAttacks.ChargeAttack}
	};

	for (const TPair<EMVCombatActionTypes, FDataTableRowHandle>& Expected : ExpectedRows)
	{
		FDataTableRowHandle Actual;
		TestTrue(TEXT("Configured basic attack resolves"), WeaponSkillMap->GetBasicAttackRow(Expected.Key, Actual));
		TestTrue(TEXT("Resolved basic attack matches configured row"), Actual == Expected.Value);
	}

	FDataTableRowHandle Unsupported = WeaponSkillMap->BasicAttacks.LightAttack;
	TestFalse(TEXT("Non-basic action type is rejected"), WeaponSkillMap->GetBasicAttackRow(EMVCombatActionTypes::Skill, Unsupported));
	TestTrue(TEXT("Rejected action clears output"), Unsupported.RowName.IsNone() && !Unsupported.DataTable);
	return true;
}

#endif
