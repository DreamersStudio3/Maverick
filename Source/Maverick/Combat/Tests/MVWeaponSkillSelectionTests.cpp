#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/MVWeaponComponent.h"
#include "Combat/MVAbilityBase.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Public/Tags/MVGameplayTags.h"
#include "Weapon/Skill/MVSkillSelectionData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVWeaponSkillSelectionRequestsTest,
	"Maverick.Combat.WeaponSkillSelection.Requests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVWeaponSkillSelectionRequestsTest::RunTest(const FString& Parameters)
{
	UMVWeaponComponent* Weapons = NewObject<UMVWeaponComponent>();
	UDataTable* Skills = NewObject<UDataTable>();
	Skills->RowStruct = FMVSkillDataTableColumn::StaticStruct();
	const auto MakeSkill = [&](const TCHAR* Name, bool bPrototype)
	{
		FMVSkillDataTableColumn Row;
		Row.DisplayName = FText::FromString(Name);
		Row.ImplementationState = bPrototype ? EMVSkillImplementationState::Prototype : EMVSkillImplementationState::Ready;
		if (!bPrototype) Row.AbilityReference = UMVAbilityBase::StaticClass();
		Skills->AddRow(Name, Row);
		FDataTableRowHandle Handle;
		Handle.DataTable = Skills;
		Handle.RowName = Name;
		return Handle;
	};
	const auto Q1 = MakeSkill(TEXT("Q1"), true);
	const auto Q2 = MakeSkill(TEXT("Q2"), true);
	const auto Locked = MakeSkill(TEXT("Locked"), true);
	const auto W1 = MakeSkill(TEXT("W1"), true);
	const auto Passive = MakeSkill(TEXT("Passive"), false);
	UMVWeaponSkillMap* Map = NewObject<UMVWeaponSkillMap>();
	Map->WeaponStyle = EMVEquippedStyle::OneHand;
	Map->QSkill.Add(FMVWeaponSkillMapData(Q1));
	Map->QSkill.Add(FMVWeaponSkillMapData(Q2));
	Map->QSkill.Add(FMVWeaponSkillMapData(Locked, 5));
	Map->WSkill.Add(FMVWeaponSkillMapData(W1));
	FMVWeaponSkillMapData PassiveEntry(Passive);
	PassiveEntry.PassiveWeaponTypes.AddTag(MVGameplayTags::Weapon_Type_Katana);
	Map->PassiveSkill.Add(PassiveEntry);
	Weapons->SkillBlueprint.Add(Map->WeaponStyle, Map);
	FMVSkillTreeState& State = Weapons->SkillTreeStates.Add(Map->WeaponStyle);
	State.EquippedStyle = Map->WeaponStyle;
	MVSkillSelectionData::InitializeActiveSlot(*Map, EMVSkillSlot::QSkill, State);
	MVSkillSelectionData::InitializeActiveSlot(*Map, EMVSkillSlot::WSkill, State);
	State.UnlockedSkills.Add(FMVHashDataTableRowHandle(Passive));
	Weapons->SkillTreeStates.Add(EMVEquippedStyle::TwoHand).EquippedStyle = EMVEquippedStyle::TwoHand;
	Weapons->SkillBlueprint.Add(EMVEquippedStyle::TwoHand, Map);

	UDataTable* WeaponTable = NewObject<UDataTable>();
	WeaponTable->RowStruct = FMVWeaponTableRow::StaticStruct();
	FMVWeaponTableRow Sword;
	Sword.ItemTag = MVGameplayTags::Item_Weapon_OneHand_TestSword;
	Sword.WeaponTypeTag = MVGameplayTags::Weapon_Type_Katana;
	Sword.EquippedStyle = Map->WeaponStyle;
	WeaponTable->AddRow(TEXT("Sword"), Sword);
	FDataTableRowHandle SwordRow;
	SwordRow.DataTable = WeaponTable;
	SwordRow.RowName = TEXT("Sword");
	TestTrue(TEXT("Acquire owned sword without save subsystem"), Weapons->AcquireWeapon(SwordRow));
	FMVWeaponTableRow Other = Sword;
	Other.ItemTag = MVGameplayTags::Item_Weapon_DualSword_TestSword;
	WeaponTable->AddRow(TEXT("Other"), Other);
	FDataTableRowHandle OtherRow = SwordRow;
	OtherRow.RowName = TEXT("Other");
	Weapons->AcquireWeapon(OtherRow);
	FMVSkillSelectionContext Context;
	Context.ItemTag = Sword.ItemTag;
	Context.WeaponType = Sword.WeaponTypeTag;
	Context.Style = Sword.EquippedStyle;
	Context.Slot = EMVSkillSlot::QSkill;
	int32 Events = 0;
	FMVSkillSelectionContext LastScope;
	const FDelegateHandle Listener = Weapons->OnSkillSelectionChanged.AddLambda([&](const auto& Scope) { ++Events; LastScope = Scope; });
	const auto InitialWeapon = Weapons->GetEquippedWeaponState();
	const auto Candidates = Weapons->GetSkillCandidates(Context);
	TestEqual(TEXT("Q candidate count includes locked row"), Candidates.Num(), 3);
	Sword.WeaponTypeTag = {};
	WeaponTable->AddRow(TEXT("Sword"), Sword);
	FMVSkillSelectionContext WithoutType = Context;
	WithoutType.WeaponType = {};
	TestEqual(TEXT("Active candidates do not require a passive type tag"), Weapons->GetSkillCandidates(WithoutType).Num(), 3);
	TestTrue(TEXT("Active request still checks unlock without a type"), Weapons->TrySelectSkill(WithoutType, Locked).Status == EMVSkillSelectionStatus::Locked);
	WithoutType.Slot = EMVSkillSlot::PassiveSkill;
	TestTrue(TEXT("Passive still requires a type"), Weapons->TrySelectSkill(WithoutType, Passive).Status == EMVSkillSelectionStatus::InvalidContext);
	Sword.WeaponTypeTag = Context.WeaponType;
	WeaponTable->AddRow(TEXT("Sword"), Sword);
	TestEqual(TEXT("Query has no change event"), Events, 0);
	TestTrue(TEXT("Select noncurrent style prototype"), Weapons->TrySelectSkill(Context, Q1).Status == EMVSkillSelectionStatus::Changed);
	TestEqual(TEXT("One event for one change"), Events, 1);
	TestTrue(TEXT("Q event scoped by style, not item or type"), LastScope.Style == Context.Style && LastScope.Slot == Context.Slot && !LastScope.ItemTag.IsValid() && !LastScope.WeaponType.IsValid());
	TestTrue(TEXT("Repeat request unchanged success"), Weapons->TrySelectSkill(Context, Q1).Status == EMVSkillSelectionStatus::Unchanged);
	TestEqual(TEXT("Repeat does not broadcast"), Events, 1);
	TestTrue(TEXT("Locked candidate rejected"), Weapons->TrySelectSkill(Context, Locked).Status == EMVSkillSelectionStatus::Locked);
	TestTrue(TEXT("Wrong slot rejected"), Weapons->TrySelectSkill(Context, W1).Status == EMVSkillSelectionStatus::InvalidCandidate);
	TestTrue(TEXT("Active clear rejected"), Weapons->TrySelectSkill(Context, {}).Status == EMVSkillSelectionStatus::InvalidCandidate);
	FMVSkillSelectionContext OtherContext = Context;
	OtherContext.ItemTag = Other.ItemTag;
	FDataTableRowHandle Selected;
	TestTrue(TEXT("Same style shares Q choice"), Weapons->GetSelectedSkill(OtherContext, Selected) && Selected == Q1);
	OtherContext.Slot = EMVSkillSlot::PassiveSkill;
	Context.Slot = EMVSkillSlot::PassiveSkill;
	TestTrue(TEXT("Passive selection delegates"), Weapons->TrySelectSkill(Context, Passive).IsSuccess());
	TestTrue(TEXT("Same type shares passive"), Weapons->GetSelectedSkill(OtherContext, Selected) && Selected == Passive);
	TestTrue(TEXT("Passive event contains type"), LastScope.WeaponType == Sword.WeaponTypeTag);
	TestTrue(TEXT("Passive clear delegates"), Weapons->TrySelectSkill(Context, {}).IsSuccess());
	TestFalse(TEXT("Passive clear removes selection"), Weapons->GetSelectedSkill(OtherContext, Selected));
	Other.WeaponTypeTag = MVGameplayTags::Weapon_Type_Rapier;
	WeaponTable->AddRow(TEXT("Other"), Other);
	OtherContext.WeaponType = Other.WeaponTypeTag;
	TestEqual(TEXT("Passive candidates filtered by type"), Weapons->GetSkillCandidates(OtherContext).Num(), 0);
	OtherContext.Slot = EMVSkillSlot::QSkill;
	TestTrue(TEXT("Different type same style shares active"), Weapons->GetSelectedSkill(OtherContext, Selected) && Selected == Q1);
	Other.EquippedStyle = EMVEquippedStyle::TwoHand;
	WeaponTable->AddRow(TEXT("Other"), Other);
	TestTrue(TEXT("Stale context rejected"), Weapons->TrySelectSkill(OtherContext, Q2).Status == EMVSkillSelectionStatus::ContextMismatch);
	OtherContext.Style = Other.EquippedStyle;
	TestFalse(TEXT("Different style starts separate"), Weapons->GetSelectedSkill(OtherContext, Selected));
	Weapons->SkillTreeStates.FindChecked(OtherContext.Style).UnlockedSkills.Add(FMVHashDataTableRowHandle(Q2));
	TestTrue(TEXT("Different style selects independently"), Weapons->TrySelectSkill(OtherContext, Q2).IsSuccess());
	Context.Slot = EMVSkillSlot::QSkill;
	TestTrue(TEXT("Original Q selection preserved"), Weapons->GetSelectedSkill(Context, Selected) && Selected == Q1);
	Weapons->CurrentWeaponState.EquippedStyle = Context.Style;
	Weapons->CurrentWeaponState.bValid = true;
	const int32 BeforeReject = Events;
	TestTrue(TEXT("Current style without combat bridge fails safely"), Weapons->TrySelectSkill(Context, Q2).Status == EMVSkillSelectionStatus::ApplyFailed);
	TestTrue(TEXT("Runtime reject preserves choice"), Weapons->GetSelectedSkill(Context, Selected) && Selected == Q1);
	TestEqual(TEXT("Rejected request has no event"), Events, BeforeReject);
	Weapons->OwnedWeapons.Remove(Context.ItemTag);
	TestTrue(TEXT("Ownership rechecked at request"), Weapons->TrySelectSkill(Context, Q2).Status == EMVSkillSelectionStatus::WeaponNotOwned);
	TestEqual(TEXT("Stale ownership query empty"), Weapons->GetSkillCandidates(Context).Num(), 0);
	TestTrue(TEXT("Request never changes equipped item"), Weapons->GetEquippedWeaponState().ItemTag == InitialWeapon.ItemTag);
	TestFalse(TEXT("Other slot not auto selected"), Weapons->SkillTreeStates.FindChecked(Context.Style).EquippedSkill.Contains(EMVSkillSlot::WSkill));
	TestEqual(TEXT("Selection did not spend points"), Weapons->GetSkillPoints(), 0);
	Weapons->OnSkillSelectionChanged.Remove(Listener);
	return true;
}

#endif
