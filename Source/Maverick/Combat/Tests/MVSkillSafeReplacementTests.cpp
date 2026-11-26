#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Combat/MVAbilityBase.h"
#include "Combat/MVProjectileExecutionTracker.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Public/Tags/MVGameplayTags.h"
#include "Weapon/Skill/MVSkillExecutionData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVSkillSafeReplacementTest,
	"Maverick.Combat.WeaponSkillSelection.SafeReplacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVSkillSafeReplacementTest::RunTest(const FString& Parameters)
{
	const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false)
		.CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
	if (!TestNotNull(TEXT("Isolated world"), World)) return false;
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	AActor* Actor = World->SpawnActor<AActor>();
	UMVCombatComponent* Combat = NewObject<UMVCombatComponent>(Actor);
	UMVWeaponComponent* Weapons = NewObject<UMVWeaponComponent>(Actor);
	Actor->AddInstanceComponent(Combat);
	Actor->AddInstanceComponent(Weapons);
	Combat->CurrentWeaponStyle = EMVEquippedStyle::OneHand;
	Combat->ProjectileExecutionTracker = NewObject<UMVProjectileExecutionTracker>(Combat);
	Combat->CurrentRSkillGauge = 61.0f;
	UDataTable* Table = NewObject<UDataTable>();
	Table->RowStruct = FMVSkillDataTableColumn::StaticStruct();
	const auto AddRow = [&](const TCHAR* Name, FMVSkillDataTableColumn Data)
	{
		Table->AddRow(Name, Data);
		FDataTableRowHandle Row;
		Row.DataTable = Table;
		Row.RowName = Name;
		return Row;
	};
	FMVSkillDataTableColumn Ready;
	Ready.AbilityReference = UMVAbilityBase::StaticClass();
	Ready.CooldownDuration = 10.0f;
	const auto A = AddRow(TEXT("A"), Ready);
	const auto B = AddRow(TEXT("B"), Ready);
	FMVSkillDataTableColumn Prototype = Ready;
	Prototype.ImplementationState = EMVSkillImplementationState::Prototype;
	Prototype.HpCost = Prototype.MpCost = Prototype.StaminaCost = 10.0f;
	const auto P = AddRow(TEXT("PrototypeWithAbility"), Prototype);
	FMVSkillDataTableColumn Broken = Ready;
	Broken.bIsChained = true;
	Broken.NextChainName = TEXT("Missing");
	const auto Missing = AddRow(TEXT("MissingChain"), Broken);
	Broken.NextChainName = TEXT("Cycle");
	const auto Cycle = AddRow(TEXT("Cycle"), Broken);
	Broken.NextChainName = TEXT("PrototypeWithAbility");
	const auto Mixed = AddRow(TEXT("Mixed"), Broken);
	TArray<FName> Rows;
	TestFalse(TEXT("Missing tail rejected"), MVSkillExecutionData::ValidateDefinition(Missing, Rows));
	TestEqual(TEXT("No partial chain on failure"), Rows.Num(), 0);
	TestFalse(TEXT("Cycle rejected"), MVSkillExecutionData::ValidateDefinition(Cycle, Rows));
	TestFalse(TEXT("Prototype inside Ready chain rejected"), MVSkillExecutionData::ValidateDefinition(Mixed, Rows));
	TestTrue(TEXT("First runtime slot prepared"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, A) == EMVSkillSelectionStatus::Changed);
	const uint64 Revision = Combat->SkillMap[EMVSkillSlot::QSkill].LoadoutRevision;
	UMVAbilityBase* Original = Combat->SkillMap[EMVSkillSlot::QSkill].GetCurrentAbility();
	TestTrue(TEXT("Repeat preserves instance"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, A) == EMVSkillSelectionStatus::Unchanged && Combat->SkillMap[EMVSkillSlot::QSkill].GetCurrentAbility() == Original);
	TestTrue(TEXT("Bad chain does not replace"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, Missing) == EMVSkillSelectionStatus::InvalidExecutionData);
	TestTrue(TEXT("Old runtime retained"), Combat->SkillMap[EMVSkillSlot::QSkill].LoadoutRevision == Revision && Combat->SkillMap[EMVSkillSlot::QSkill].GetCurrentAbility() == Original);
	Combat->SkillMap[EMVSkillSlot::QSkill].MainCooldownEndTime = World->GetTimeSeconds() + 8.0f;
	TestTrue(TEXT("Cooldown blocks replacement"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, B) == EMVSkillSelectionStatus::CooldownActive);
	Combat->SkillMap[EMVSkillSlot::QSkill].MainCooldownEndTime = -1.0f;
	Combat->SkillMap[EMVSkillSlot::QSkill].bChainActive = true;
	Combat->SkillMap[EMVSkillSlot::QSkill].InputWindowCloseTime = World->GetTimeSeconds() + 2.0f;
	TestTrue(TEXT("Chain blocks replacement"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, B) == EMVSkillSelectionStatus::ChainActive);
	Combat->SkillMap[EMVSkillSlot::QSkill].bChainActive = false;
	Combat->CurrentPreparedSkillSlot = EMVSkillSlot::QSkill;
	Combat->bCurrentAbilityAwaitingCompletion = true;
	TestTrue(TEXT("Pending action blocks replacement"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, B) == EMVSkillSelectionStatus::ActionRunning);
	Combat->bCurrentAbilityAwaitingCompletion = false;
	TestTrue(TEXT("Other slot prepared"), Combat->TryReplaceSkillSlot(EMVSkillSlot::WSkill, B) == EMVSkillSelectionStatus::Changed);
	const uint64 OtherRevision = Combat->SkillMap[EMVSkillSlot::WSkill].LoadoutRevision;
	TestTrue(TEXT("Prototype prepares without ability"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, P) == EMVSkillSelectionStatus::Changed);
	TestEqual(TEXT("Prototype creates no Ability despite reference"), Combat->SkillMap[EMVSkillSlot::QSkill].AbilityInstances.Num(), 0);
	TestFalse(TEXT("Prototype direct use blocked"), Combat->TrySkill(EMVSkillSlot::QSkill));
	TestFalse(TEXT("Prototype combat use blocked"), Combat->TrySkillCombatAction(EMVSkillSlot::QSkill));
	TestTrue(TEXT("Prototype tag input consumed"), Combat->ChooseTryCombatAction(MVGameplayTags::Action_Input_Skill_Q));
	UMVInputManagerComponent* Input = NewObject<UMVInputManagerComponent>(Actor);
	Input->RegisterActionInputHandler(Combat, 0);
	TestTrue(TEXT("Input manager accepts and consumes prototype input"), Input->SubmitActionInput(MVGameplayTags::Action_Input_Skill_Q));
	FGameplayTag BufferedTag;
	FVector2D BufferedMovement;
	bool bBufferedMovement = false;
	TestFalse(TEXT("Prototype leaves no queued input"), Input->TryGetBufferedActionInput(BufferedTag, BufferedMovement, bBufferedMovement));
	TestFalse(TEXT("Prototype direct row use blocked"), Combat->TryStartCombatActionFromRowHandle(P));
	TestTrue(TEXT("Prototype costs blocked before consumption"), !Combat->CanConsumeActionCost(&Prototype));
	TestTrue(TEXT("Other slot untouched"), Combat->SkillMap[EMVSkillSlot::WSkill].LoadoutRevision == OtherRevision);
	TestEqual(TEXT("R gauge preserved"), Combat->CurrentRSkillGauge, 61.0f);
	TestTrue(TEXT("No prototype cooldown"), Combat->SkillMap[EMVSkillSlot::QSkill].MainCooldownEndTime < 0.0f);
	TestTrue(TEXT("Ready replacement succeeds after Prototype"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, B) == EMVSkillSelectionStatus::Changed && Combat->IsSkillSlotExecutable(EMVSkillSlot::QSkill));
	TestFalse(TEXT("Ready replacement cannot replay prototype input"), Input->TryGetBufferedActionInput(BufferedTag, BufferedMovement, bBufferedMovement));
	Combat->SkillMap[EMVSkillSlot::QSkill].bChainActive = true;
	Combat->SkillMap[EMVSkillSlot::QSkill].InputWindowCloseTime = -1.0f;
	TestTrue(TEXT("Expired chain does not block forever"), Combat->TryReplaceSkillSlot(EMVSkillSlot::QSkill, A) == EMVSkillSelectionStatus::Changed);

	UMVWeaponSkillMap* Map = NewObject<UMVWeaponSkillMap>();
	Map->WeaponStyle = EMVEquippedStyle::OneHand;
	for (const auto& Row : {A, B, P, Missing}) Map->QSkill.Add(FMVWeaponSkillMapData(Row));
	Weapons->SkillBlueprint.Add(Map->WeaponStyle, Map);
	auto& State = Weapons->SkillTreeStates.Add(Map->WeaponStyle);
	State.EquippedStyle = Map->WeaponStyle;
	for (const auto& Row : {A, B, P, Missing}) State.UnlockedSkills.Add(FMVHashDataTableRowHandle(Row));
	State.EquippedSkill.Add(EMVSkillSlot::QSkill, B);
	Weapons->CurrentWeaponState.bValid = true;
	Weapons->CurrentWeaponState.EquippedStyle = Map->WeaponStyle;
	TestTrue(TEXT("Legacy current equip uses safe bridge"), Weapons->EquipSkill(Map->WeaponStyle, A));
	TestTrue(TEXT("Selection and runtime commit together"), State.EquippedSkill[EMVSkillSlot::QSkill] == A && Combat->SkillMap[EMVSkillSlot::QSkill].GetCurrentAbility() != Original);
	TestFalse(TEXT("Legacy invalid equip rejected"), Weapons->EquipSkill(Map->WeaponStyle, Missing));
	TestTrue(TEXT("Failure preserves both records"), State.EquippedSkill[EMVSkillSlot::QSkill] == A && Combat->SkillMap[EMVSkillSlot::QSkill].SkillRowNames[0] == A.RowName);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
