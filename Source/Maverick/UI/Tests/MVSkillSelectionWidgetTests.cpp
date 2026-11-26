#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "CommonUITypes.h"
#include "Engine/World.h"
#include "UI/System/MVCommonUIInputData.h"
#include "UI/Window/MVWeaponPassiveSelectionWindow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVSkillSelectionWidgetContractTest, "Maverick.UI.SkillSelection.AssetAndBackContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVSkillSelectionWidgetContractTest::RunTest(const FString& Parameters)
{
	UClass* Class = LoadClass<UMVWeaponSkillSelectionWindow>(nullptr, TEXT("/Game/UI/Windows/WBP_WeaponPassiveSelection.WBP_WeaponPassiveSelection_C"));
	if (!TestNotNull(TEXT("Existing widget asset loads as common selection window"), Class)) return false;
	TestTrue(TEXT("Legacy type remains compatible"), Class->IsChildOf(UMVWeaponPassiveSelectionWindow::StaticClass()));
	const auto* Generated = Cast<UWidgetBlueprintGeneratedClass>(Class);
	UWidgetTree* Tree = Generated ? Generated->GetWidgetTreeArchetype() : nullptr;
	if (!TestNotNull(TEXT("Authored widget tree"), Tree)) return false;
	TestNotNull(TEXT("Tab host"), Cast<UHorizontalBox>(Tree->FindWidget(TEXT("SlotTabs"))));
	TestNotNull(TEXT("Sharing scope label"), Cast<UTextBlock>(Tree->FindWidget(TEXT("SharingScopeText"))));
	TestNotNull(TEXT("Save retry button"), Cast<UButton>(Tree->FindWidget(TEXT("RetrySaveButton"))));
	TestNotNull(TEXT("Apply button retained"), Cast<UButton>(Tree->FindWidget(TEXT("ApplyButton"))));
	TestNull(TEXT("Close button remains removed"), Tree->FindWidget(TEXT("CloseButton")));
	TestTrue(TEXT("Retry hidden before a failure"), Tree->FindWidget(TEXT("RetrySaveButton"))->GetVisibility() == ESlateVisibility::Collapsed);
	UWorld* PIE = NewObject<UWorld>();
	PIE->WorldType = EWorldType::PIE;
	UWorld* Game = NewObject<UWorld>();
	Game->WorldType = EWorldType::Game;
	auto* Input = GetMutableDefault<UMVCommonUIInputData>();
	const auto PIEBack = Input->GetBackActionForWorld(PIE);
	const auto GameBack = Input->GetBackActionForWorld(Game);
	const auto* PIEData = PIEBack.GetRow<FCommonInputActionDataBase>(TEXT("SkillSelection.PIEBack"));
	const auto* GameData = GameBack.GetRow<FCommonInputActionDataBase>(TEXT("SkillSelection.GameBack"));
	if (TestNotNull(TEXT("PIE back mapping"), PIEData) && TestNotNull(TEXT("Game back mapping"), GameData))
	{
		TestEqual(TEXT("PIE back uses Backspace"), PIEData->GetInputTypeInfo(ECommonInputType::MouseAndKeyboard, NAME_None).GetKey(), EKeys::BackSpace);
		TestEqual(TEXT("Game back uses Escape"), GameData->GetInputTypeInfo(ECommonInputType::MouseAndKeyboard, NAME_None).GetKey(), EKeys::Escape);
	}
	return !HasAnyErrors();
}
#endif
