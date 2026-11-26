#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/GameInstance.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVWeaponComponent.h"
#include "Components/MVInventoryComponent.h"
#include "System/MVWorldStateSubsystem.h"
#include "Public/Tags/MVGameplayTags.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Weapon/Skill/MVSkillSelectionData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVSkillSelectionPersistenceTest,
	"Maverick.Combat.WeaponSkillSelection.Persistence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVSkillSelectionPersistenceTest::RunTest(const FString& Parameters)
{
	const FString Manifest = FPaths::ProjectSavedDir() / TEXT("Logs/SkillSelectionStage4/RestartSlot.txt");
	const auto* Fixture = LoadObject<UMVWeaponSkillMap>(nullptr, TEXT("/Game/Tests/SkillSelection/DA_SkillSelectionPrototypes.DA_SkillSelectionPrototypes"));
	if (!TestNotNull(TEXT("Persistent prototype fixture"), Fixture)) return false;
	const auto MakeWeapons = [&]()
	{
		auto* Weapons = NewObject<UMVWeaponComponent>();
		Weapons->Inventory = NewObject<UMVInventoryComponent>(Weapons);
		auto* Map = DuplicateObject<UMVWeaponSkillMap>(Fixture, Weapons);
		Map->WeaponStyle = EMVEquippedStyle::OneHand;
		Weapons->SkillBlueprint.Add(Map->WeaponStyle, Map);
		Weapons->SkillTreeStates.Add(Map->WeaponStyle).EquippedStyle = Map->WeaponStyle;
		Weapons->InitSkillTreeStates();
		return Weapons;
	};
	// Initialize를 호출하지 않아 사용자 기본 슬롯을 읽거나 생성하지 않는다.
	const auto MakeSave = []()
	{
		auto* Save = NewObject<UMVWorldStateSubsystem>(NewObject<UGameInstance>());
		Save->ResetSaveData();
		return Save;
	};
	if (FParse::Param(FCommandLine::Get(), TEXT("SkillSelectionReadRestart")))
	{
		FString Slot;
		if (!TestTrue(TEXT("Writer manifest exists"), FFileHelper::LoadFileToString(Slot, *Manifest))) return false;
		if (!TestTrue(TEXT("Only test slot allowed"), Slot.StartsWith(TEXT("CodexSkillSelection_")) && !Slot.Contains(TEXT("/")) && !Slot.Contains(TEXT("\\")))) return false;
		auto* Save = MakeSave();
		if (!TestTrue(TEXT("Fresh process loads disk"), Save->LoadFromSlot(Slot, 0))) return false;
		auto* Weapons = MakeWeapons();
		Weapons->RestoreWeaponProgress(Save->GetWeaponProgress(), Save->GetSaveData().Inventory);
		TestEqual(TEXT("Save version is five"), Save->GetSaveData().SaveVersion, 5);
		TestTrue(TEXT("Empty inventory survives restart"),
			Save->GetSaveData().Inventory.bHasRecord && Weapons->Inventory->GetItems().IsEmpty());
		TestTrue(TEXT("Other world state survives"), Save->GetWorldFlag(TEXT("SkillSelectionSentinel"), false));
		for (const auto& SlotDefinition : MVSkillSelectionData::GetSlots())
		{
			if (!MVSkillSelectionData::IsActiveSlot(SlotDefinition.Slot)) continue;
			FDataTableRowHandle Row;
			const auto Expected = Fixture->GetSortedSkillCandidates(SlotDefinition.Slot)[1].SkillRowHandle.RowHandle;
			TestTrue(TEXT("Fresh process restores each Prototype choice"), Weapons->GetEquippedSkillRow(EMVEquippedStyle::OneHand, SlotDefinition.Slot, Row) && Row == Expected);
		}
		if (!HasAnyErrors()) TestTrue(TEXT("Delete only generated restart slot"), Save->DeleteSaveSlot(Slot, 0));
		return !HasAnyErrors();
	}

	auto* Weapons = MakeWeapons();
	TestTrue(TEXT("Missing subsystem is explicit"), Weapons->RetrySkillSelectionSave() == EMVSkillSelectionSaveStatus::Unavailable);
	auto* Save = MakeSave();
	const FString SlotName = TEXT("CodexSkillSelection_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	Save->ActiveSaveSlotName = SlotName;
	Weapons->WeaponSaveState = Save;
	Save->SetWorldFlag(TEXT("SkillSelectionSentinel"), true);
	const FString FailureDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("SaveGames") / (SlotName + TEXT(".sav")));
	const FString AllowedRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("SaveGames")) + TEXT("/");
	if (!TestTrue(TEXT("Failure fixture stays inside SaveGames"), FailureDirectory.StartsWith(AllowedRoot))) return false;
	if (!TestTrue(TEXT("Unique test target absent"), !IFileManager::Get().FileExists(*FailureDirectory) && !IFileManager::Get().DirectoryExists(*FailureDirectory))) return false;
	TestTrue(TEXT("Create directory at test file path"), IFileManager::Get().MakeDirectory(*FailureDirectory, true));
	UDataTable* WeaponTable = NewObject<UDataTable>();
	WeaponTable->RowStruct = FMVWeaponTableRow::StaticStruct();
	FMVWeaponTableRow Weapon;
	Weapon.ItemTag = MVGameplayTags::Item_Weapon_OneHand_TestSword;
	Weapon.WeaponTypeTag = MVGameplayTags::Weapon_Type_Katana;
	Weapon.EquippedStyle = EMVEquippedStyle::OneHand;
	WeaponTable->AddRow(TEXT("Owned"), Weapon);
	FDataTableRowHandle WeaponRow;
	WeaponRow.DataTable = WeaponTable;
	WeaponRow.RowName = TEXT("Owned");
	TestTrue(TEXT("Acquire fixture sword"), Weapons->AcquireWeapon(WeaponRow));
	FMVSkillSelectionContext Context;
	Context.ItemTag = Weapon.ItemTag;
	Context.WeaponType = Weapon.WeaponTypeTag;
	Context.Style = Weapon.EquippedStyle;
	Context.Slot = EMVSkillSlot::QSkill;
	int32 Events = 0;
	Weapons->OnSkillSelectionChanged.AddLambda([&](const auto&) { ++Events; });
	const auto Q = Fixture->GetSortedSkillCandidates(Context.Slot)[1].SkillRowHandle.RowHandle;
	const auto Failed = Weapons->TrySelectSkill(Context, Q);
	TestTrue(TEXT("Selection succeeds despite failed disk write"), Failed.IsSuccess() && Failed.bSelectionChanged && Failed.SaveStatus == EMVSkillSelectionSaveStatus::Failed);
	FDataTableRowHandle Selected;
	TestTrue(TEXT("Failed write keeps selected row"), Weapons->GetSelectedSkill(Context, Selected) && Selected == Q);
	TestTrue(TEXT("World state records write failure"), Save->HasFailedActiveSave());
	TestTrue(TEXT("Remove only empty test directory"), IFileManager::Get().DeleteDirectory(*FailureDirectory, false, false));
	const int32 BeforeRetry = Events;
	TestTrue(TEXT("Retry saves current state only"), Weapons->RetrySkillSelectionSave() == EMVSkillSelectionSaveStatus::Saved);
	TestEqual(TEXT("Retry emits no selection event"), Events, BeforeRetry);
	TestFalse(TEXT("Retry clears failed save state"), Save->HasFailedActiveSave());
	TestTrue(TEXT("Retry did not reselect"), Weapons->GetSelectedSkill(Context, Selected) && Selected == Q);
	const auto Same = Weapons->TrySelectSkill(Context, Q);
	TestTrue(TEXT("Same choice does not write again"), Same.Status == EMVSkillSelectionStatus::Unchanged && Same.SaveStatus == EMVSkillSelectionSaveStatus::NotRequested);
	for (const auto& Slot : MVSkillSelectionData::GetSlots())
	{
		if (!MVSkillSelectionData::IsActiveSlot(Slot.Slot)) continue;
		Context.Slot = Slot.Slot;
		const auto Row = Fixture->GetSortedSkillCandidates(Slot.Slot)[1].SkillRowHandle.RowHandle;
		if (Slot.Slot != EMVSkillSlot::QSkill)
			TestTrue(TEXT("Each choice reports disk success"), Weapons->TrySelectSkill(Context, Row).SaveStatus == EMVSkillSelectionSaveStatus::Saved);
	}
	// 준비된 정상 패시브는 임시 표로 검사, 프로세스 재시작 검사는 영구 자산인 QWER로 한정한다.
	UDataTable* Passives = NewObject<UDataTable>();
	Passives->RowStruct = FMVSkillDataTableColumn::StaticStruct();
	FMVSkillDataTableColumn PassiveData;
	PassiveData.AbilityReference = UMVAbilityBase::StaticClass();
	Passives->AddRow(TEXT("Passive"), PassiveData);
	FDataTableRowHandle Passive;
	Passive.DataTable = Passives;
	Passive.RowName = TEXT("Passive");
	auto* Map = Weapons->SkillBlueprint.FindChecked(EMVEquippedStyle::OneHand).Get();
	FMVWeaponSkillMapData Entry(Passive);
	Entry.PassiveWeaponTypes.AddTag(Weapon.WeaponTypeTag);
	Map->PassiveSkill.Add(Entry);
	Weapons->SkillTreeStates.FindChecked(EMVEquippedStyle::OneHand).UnlockedSkills.Add(FMVHashDataTableRowHandle(Passive));
	Context.Slot = EMVSkillSlot::PassiveSkill;
	TestTrue(TEXT("Passive reports save success"), Weapons->TrySelectSkill(Context, Passive).SaveStatus == EMVSkillSelectionSaveStatus::Saved);
	auto* Loaded = MakeSave();
	TestTrue(TEXT("New subsystem reloads disk"), Loaded->LoadFromSlot(SlotName, 0));
	const FMVInventorySaveData SavedItems = Loaded->GetSaveData().Inventory;
	Weapons->RestoreWeaponProgress(Loaded->GetWeaponProgress(), SavedItems);
	TestTrue(TEXT("Passive restored"), Weapons->GetSelectedPassive(Weapon.WeaponTypeTag, Selected) && Selected == Passive);
	TestTrue(TEXT("Owned weapon preserved"), Weapons->IsWeaponOwned(Weapon.ItemTag));
	TestTrue(TEXT("Unrelated flag preserved"), Loaded->GetWorldFlag(TEXT("SkillSelectionSentinel"), false));
	{
		TGuardValue<bool> Guard(Weapons->bRestoringWeaponProgress, true);
		TestTrue(TEXT("Restore suppresses save retries"), Weapons->RetrySkillSelectionSave() == EMVSkillSelectionSaveStatus::SuppressedDuringRestore);
	}
	// 같은 저장 형식의 행 누락과 슬롯 이동을 별도 복원 자료로 검사한다.
	FMVWeaponProgressSaveData Invalid = Loaded->GetWeaponProgress();
	auto& SavedStyle = Invalid.SkillStyles.FindChecked(EMVEquippedStyle::OneHand);
	SavedStyle.EquippedRows.FindChecked(EMVSkillSlot::QSkill).RowName = TEXT("RemovedRow");
	SavedStyle.EquippedRows.FindChecked(EMVSkillSlot::WSkill) = SavedStyle.EquippedRows.FindChecked(EMVSkillSlot::ESkill);
	Weapons->RestoreWeaponProgress(Invalid, SavedItems);
	TestFalse(TEXT("Removed Prototype falls back to empty"), Weapons->GetEquippedSkillRow(EMVEquippedStyle::OneHand, EMVSkillSlot::QSkill, Selected));
	TestFalse(TEXT("Moved slot is rejected"), Weapons->GetEquippedSkillRow(EMVEquippedStyle::OneHand, EMVSkillSlot::WSkill, Selected));
	TestTrue(TEXT("Other slot retained"), Weapons->GetEquippedSkillRow(EMVEquippedStyle::OneHand, EMVSkillSlot::RSkill, Selected));
	TestTrue(TEXT("Passive unaffected by bad active rows"), Weapons->GetSelectedPassive(Weapon.WeaponTypeTag, Selected) && Selected == Passive);
	const auto AddReadyCandidate = [&](const FName Name, int32 Cost, bool bBroken)
	{
		FMVSkillDataTableColumn Data = PassiveData;
		Data.bIsChained = bBroken;
		Data.NextChainName = bBroken ? FName(TEXT("MissingTail")) : NAME_None;
		Passives->AddRow(Name, Data);
		FDataTableRowHandle Row;
		Row.DataTable = Passives;
		Row.RowName = Name;
		Map->QSkill.Add(FMVWeaponSkillMapData(Row, Cost));
		return Row;
	};
	const auto AReady = AddReadyCandidate(TEXT("AReady"), 0, false);
	AddReadyCandidate(TEXT("ZReady"), 0, false);
	const auto Broken = AddReadyCandidate(TEXT("BrokenReady"), 0, true);
	const auto Paid = AddReadyCandidate(TEXT("PaidReady"), 5, false);
	Map->DefaultSkillRows.Add(EMVSkillSlot::QSkill, Broken);
	for (int32 Repeat = 0; Repeat < 3; ++Repeat)
	{
		Weapons->RestoreWeaponProgress(Invalid, SavedItems);
		TestTrue(TEXT("Invalid default uses deterministic complete Ready fallback"), Weapons->GetEquippedSkillRow(EMVEquippedStyle::OneHand, EMVSkillSlot::QSkill, Selected) && Selected == AReady);
	}
	FMVSavedWeaponRow PaidKey;
	PaidKey.Table = Passives;
	PaidKey.RowName = Paid.RowName;
	SavedStyle.EquippedRows.FindChecked(EMVSkillSlot::QSkill) = PaidKey;
	Weapons->RestoreWeaponProgress(Invalid, SavedItems);
	TestTrue(TEXT("Unrecorded paid unlock cannot restore"), Weapons->GetEquippedSkillRow(EMVEquippedStyle::OneHand, EMVSkillSlot::QSkill, Selected) && Selected == AReady);
	SavedStyle.UnlockedRows.Add(PaidKey);
	Weapons->RestoreWeaponProgress(Invalid, SavedItems);
	TestTrue(TEXT("Recorded paid unlock restores without spending"), Weapons->GetEquippedSkillRow(EMVEquippedStyle::OneHand, EMVSkillSlot::QSkill, Selected) && Selected == Paid && Weapons->GetSkillPoints() == Invalid.SkillPoints);
	FMVSkillDataTableColumn InvalidPassive = PassiveData;
	InvalidPassive.ImplementationState = EMVSkillImplementationState::Prototype;
	Passives->AddRow(TEXT("Passive"), InvalidPassive);
	Weapons->RestoreWeaponProgress(Invalid, SavedItems);
	TestFalse(TEXT("Prototype passive cannot restore even with an Ability"), Weapons->GetSelectedPassive(Weapon.WeaponTypeTag, Selected));
	Passives->AddRow(TEXT("Passive"), PassiveData);

	// 새 프로세스가 읽을 수 있는 영구 행만 마지막에 저장한다.
	FMVWeaponProgressSaveData Restart = Loaded->GetWeaponProgress();
	FMVInventorySaveData EmptyInventory;
	EmptyInventory.bHasRecord = true;
	Restart.SelectedPassives.Reset();
	for (auto& Style : Restart.SkillStyles)
	{
		Style.Value.UnlockedRows.RemoveAll([](const auto& Key) { return !Key.Table.ToSoftObjectPath().GetLongPackageName().StartsWith(TEXT("/Game/")); });
	}
	Save->SetPlayerItemProgress(MoveTemp(EmptyInventory), MoveTemp(Restart));
	TestTrue(TEXT("Write durable restart fixture"), Save->SaveActiveSlot());
	TestTrue(TEXT("Publish generated slot name"), FFileHelper::SaveStringToFile(SlotName, *Manifest));
	Weapons->OnSkillSelectionChanged.Clear();
	return !HasAnyErrors();
}

#endif
