#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "DataAsset/MVWeaponSkillMap.h"
#include "Engine/DataTable.h"
#include "Combat/MVAbilityBase.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Public/Struct/MVSkillTree.h"
#include "Weapon/Skill/MVSkillSelectionData.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVWeaponComponent.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVPlayerConfiguredSkillMapsTest,
	"Maverick.Combat.WeaponSkillMap.PlayerConfiguredAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVPlayerConfiguredSkillMapsTest::RunTest(const FString& Parameters)
{
	const UClass* PlayerClass = LoadClass<AMVPlayerCharacter>(nullptr,
		TEXT("/Game/Characters/CharacterLogic/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	if (!TestNotNull(TEXT("Actual player class"), PlayerClass)) return false;
	const auto* Player = PlayerClass->GetDefaultObject<AMVPlayerCharacter>();
	if (!TestNotNull(TEXT("Actual player weapon component"), Player->WeaponComponent.Get())) return false;
	TestTrue(TEXT("Player has authored skill maps"), !Player->WeaponComponent->WeaponSkillMaps.IsEmpty());
	TSet<EMVEquippedStyle> Styles;
	for (const UMVWeaponSkillMap* Map : Player->WeaponComponent->WeaponSkillMaps)
	{
		if (!TestNotNull(TEXT("Configured map"), Map)) continue;
		TestFalse(*FString::Printf(TEXT("Unique style: %s"), *Map->GetPathName()), Styles.Contains(Map->WeaponStyle));
		Styles.Add(Map->WeaponStyle);
		TArray<FString> Errors;
		if (!Map->ValidateSkillCandidates(Errors))
			for (const auto& Error : Errors) AddError(Map->GetPathName() + TEXT(": ") + Error);
		for (const auto& Passive : Map->PassiveSkill)
			TestFalse(*FString::Printf(TEXT("Selectable passive has weapon types: %s/%s"),
				*Map->GetPathName(), *Passive.SkillRowHandle.RowHandle.RowName.ToString()), Passive.PassiveWeaponTypes.IsEmpty());
	}
	return !HasAnyErrors();
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
	WeaponSkillMap->BasicAttacks.ChargeAttack = MVWeaponSkillMapTestMakeRow(*DataTable, TEXT("ChargeAttack"));

	const TPair<EMVCombatActionTypes, FDataTableRowHandle> ExpectedRows[] =
	{
		{EMVCombatActionTypes::LightAttack, WeaponSkillMap->BasicAttacks.LightAttack},
		{EMVCombatActionTypes::DodgeLightAttack, WeaponSkillMap->BasicAttacks.DodgeLightAttack},
		{EMVCombatActionTypes::SprintLightAttack, WeaponSkillMap->BasicAttacks.SprintLightAttack},
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVSkillSelectionDataContractTest,
	"Maverick.Combat.WeaponSkillMap.SelectionDataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVSkillSelectionDataContractTest::RunTest(const FString& Parameters)
{
	UDataTable* Table = NewObject<UDataTable>();
	Table->RowStruct = FMVSkillDataTableColumn::StaticStruct();
	UMVWeaponSkillMap* Map = NewObject<UMVWeaponSkillMap>();
	FMVSkillDataTableColumn Ready;
	TestTrue(TEXT("Existing rows default to Ready"), Ready.ImplementationState == EMVSkillImplementationState::Ready);
	Ready.AbilityReference = UMVAbilityBase::StaticClass();
	Table->AddRow(TEXT("ZReady"), Ready);
	Table->AddRow(TEXT("AReady"), Ready);
	FMVSkillDataTableColumn Prototype;
	Prototype.ImplementationState = EMVSkillImplementationState::Prototype;
	Table->AddRow(TEXT("Prototype"), Prototype);
	const auto A = MVWeaponSkillMapTestMakeRow(*Table, TEXT("AReady"));
	const auto Z = MVWeaponSkillMapTestMakeRow(*Table, TEXT("ZReady"));
	const auto P = MVWeaponSkillMapTestMakeRow(*Table, TEXT("Prototype"));
	Map->QSkill.Add(FMVWeaponSkillMapData(Z));
	Map->QSkill.Add(FMVWeaponSkillMapData(P));
	Map->QSkill.Add(FMVWeaponSkillMapData(A));
	TArray<FString> Errors;
	TestTrue(TEXT("Ready and explicit prototype rows valid"), Map->ValidateSkillCandidates(Errors));
	FDataTableRowHandle Default;
	for (int32 Iteration = 0; Iteration < 12; ++Iteration)
	{
		TestTrue(TEXT("Default found"), Map->GetDefaultSkillRow(EMVSkillSlot::QSkill, Default));
		TestTrue(TEXT("Sorted free Ready default is stable"), Default == A);
	}
	Map->DefaultSkillRows.Add(EMVSkillSlot::QSkill, Z);
	Map->GetDefaultSkillRow(EMVSkillSlot::QSkill, Default);
	TestTrue(TEXT("Explicit default wins"), Default == Z);
	FMVSkillTreeState State;
	State.EquippedSkill.Add(EMVSkillSlot::QSkill, P);
	MVSkillSelectionData::InitializeActiveSlot(*Map, EMVSkillSlot::QSkill, State);
	TestTrue(TEXT("Existing prototype selection preserved"), State.EquippedSkill[EMVSkillSlot::QSkill] == P);
	TestTrue(TEXT("Free prototype unlocked"), State.UnlockedSkills.Contains(FMVHashDataTableRowHandle(P)));
	Map->DefaultSkillRows.Add(EMVSkillSlot::QSkill, P);
	TestFalse(TEXT("Prototype explicit default rejected"), Map->ValidateSkillCandidates(Errors));
	Map->DefaultSkillRows.Reset();
	Map->QSkill.Remove(FMVWeaponSkillMapData(Z));
	Map->QSkill.Add(FMVWeaponSkillMapData(Z, 5));
	Map->DefaultSkillRows.Add(EMVSkillSlot::QSkill, Z);
	TestFalse(TEXT("Paid default rejected"), Map->ValidateSkillCandidates(Errors));
	Map->DefaultSkillRows.Reset();
	Map->QSkill.Remove(FMVWeaponSkillMapData(Z));
	Map->QSkill.Add(FMVWeaponSkillMapData(Z));
	Map->WSkill.Add(FMVWeaponSkillMapData(P));
	TestFalse(TEXT("Duplicate slot registration rejected"), Map->ValidateSkillCandidates(Errors));
	TestEqual(TEXT("Ambiguous row excluded"), Map->GetSortedSkillCandidates(EMVSkillSlot::QSkill).Num(), 2);
	Map->WSkill.Reset();
	Map->QSkill.Reset();
	Map->QSkill.Add(FMVWeaponSkillMapData(P));
	State = {};
	MVSkillSelectionData::InitializeActiveSlot(*Map, EMVSkillSlot::QSkill, State);
	TestFalse(TEXT("Prototype-only slot remains empty"), State.EquippedSkill.Contains(EMVSkillSlot::QSkill));
	Map->ESkill.Add(FMVWeaponSkillMapData(MVWeaponSkillMapTestMakeRow(*Table, TEXT("Missing"))));
	TestFalse(TEXT("Missing row rejected"), Map->ValidateSkillCandidates(Errors));
	Map->ESkill.Reset();
	UDataTable* WrongTable = NewObject<UDataTable>();
	WrongTable->RowStruct = FTableRowBase::StaticStruct();
	Map->ESkill.Add(FMVWeaponSkillMapData(MVWeaponSkillMapTestMakeRow(*WrongTable, TEXT("WrongType"))));
	TestFalse(TEXT("Wrong row structure rejected safely"), Map->ValidateSkillCandidates(Errors));
	Map->ESkill.Reset();
	Map->PassiveSkill.Add(FMVWeaponSkillMapData(P));
	TestFalse(TEXT("Prototype passive rejected"), Map->ValidateSkillCandidates(Errors));
	const auto& Slots = MVSkillSelectionData::GetSlots();
	TestEqual(TEXT("Five slots"), Slots.Num(), 5);
	TestTrue(TEXT("UI order uses Q W E R"), Slots[1].Slot == EMVSkillSlot::QSkill && Slots[2].Slot == EMVSkillSlot::WSkill && Slots[3].Slot == EMVSkillSlot::ESkill && Slots[4].Slot == EMVSkillSlot::RSkill);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVSkillSelectionPrototypeAssetsTest,
	"Maverick.Combat.WeaponSkillMap.PrototypeAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVSkillSelectionPrototypeAssetsTest::RunTest(const FString& Parameters)
{
	const auto* Map = LoadObject<UMVWeaponSkillMap>(nullptr, TEXT("/Game/Tests/SkillSelection/DA_SkillSelectionPrototypes.DA_SkillSelectionPrototypes"));
	if (!TestNotNull(TEXT("Isolated prototype asset loads"), Map)) return false;
	TArray<FString> Errors;
	TestTrue(TEXT("Fixture candidates validate"), Map->ValidateSkillCandidates(Errors));
	TestEqual(TEXT("No passive fixture"), Map->PassiveSkill.Num(), 0);
	FMVSkillTreeState State;
	for (const auto& Slot : MVSkillSelectionData::GetSlots())
	{
		if (!MVSkillSelectionData::IsActiveSlot(Slot.Slot)) continue;
		const auto Candidates = Map->GetSortedSkillCandidates(Slot.Slot);
		TestEqual(TEXT("Two prototypes per slot"), Candidates.Num(), 2);
		for (const auto& Candidate : Candidates)
		{
			const auto* Row = Candidate.SkillRowHandle.RowHandle.GetRow<FMVSkillDataTableColumn>(TEXT("FixtureTest"));
			if (!TestNotNull(TEXT("Fixture row exists"), Row)) continue;
			TestTrue(TEXT("Fixture explicitly Prototype"), Row->ImplementationState == EMVSkillImplementationState::Prototype);
			TestTrue(TEXT("No fake ability"), Row->AbilityReference == nullptr);
			TestFalse(TEXT("Fixture icon assigned"), Row->Icon.IsNull());
			TestNotNull(TEXT("Fixture icon loads"), Row->Icon.LoadSynchronous());
		}
		MVSkillSelectionData::InitializeActiveSlot(*Map, Slot.Slot, State);
		TestFalse(TEXT("Fixture cannot auto-equip"), State.EquippedSkill.Contains(Slot.Slot));
	}
	TestEqual(TEXT("All eight free candidates unlock"), State.UnlockedSkills.Num(), 8);
	return true;
}

#endif
