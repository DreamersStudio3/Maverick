#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Combat/MVAbilityBase.h"
#include "Engine/Texture2D.h"
#include "EnhancedActionKeyMapping.h"
#include "InputTriggers.h"
#include "Tables/MVSkillDataTableColumn.h"
#include "UI/HUD/MVSkillHUDPresentation.h"
#include "UI/HUD/MVSkillSlotWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVSkillHUDPresentationTest, "Maverick.UI.SkillHUD.SelectionAndChainIcons",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVSkillHUDPresentationTest::RunTest(const FString& Parameters)
{
	UDataTable* Table = NewObject<UDataTable>();
	Table->RowStruct = FMVSkillDataTableColumn::StaticStruct();
	UTexture2D* First = NewObject<UTexture2D>();
	UTexture2D* Last = NewObject<UTexture2D>();
	UTexture2D* Fallback = NewObject<UTexture2D>();
	FMVSkillDataTableColumn Row;
	Row.AbilityReference = UMVAbilityBase::StaticClass();
	Row.bIsChained = true;
	Row.NextChainName = TEXT("Middle");
	Row.Icon = First;
	Table->AddRow(TEXT("Start"), Row);
	Row.Icon.Reset();
	Row.NextChainName = TEXT("Last");
	Table->AddRow(TEXT("Middle"), Row);
	Row.Icon = Last;
	Row.bIsChained = false;
	Table->AddRow(TEXT("Last"), Row);
	FDataTableRowHandle Handle;
	Handle.DataTable = Table;
	Handle.RowName = TEXT("Start");
	const auto Chain = MVSkillHUDPresentation::Build(Handle, Fallback);
	if (!TestEqual(TEXT("A missing middle icon never shifts the last stage"), Chain.Icons.Num(), 3)) return false;
	TestTrue(TEXT("Each chain stage uses its own row or fallback"), Chain.Icons[0].Get() == First && Chain.Icons[1].Get() == Fallback && Chain.Icons[2].Get() == Last);
	UMVSkillSlotWidget* Widget = NewObject<UMVSkillSlotWidget>();
	Widget->SetIconStack({First, Fallback, Last});
	Widget->SetRuntimeState(2, 3, 5, true);
	TestTrue(TEXT("Last stage shown"), Widget->GetDisplayedIcon() == Last);
	Widget->SetIconStack({First});
	TestTrue(TEXT("New one-stage selection cannot retain previous chain icon"), Widget->GetDisplayedIcon() == First);
	Row.ImplementationState = EMVSkillImplementationState::Prototype;
	Row.AbilityReference = nullptr;
	Row.bIsChained = true;
	Row.NextChainName = TEXT("Missing");
	Table->AddRow(TEXT("Prototype"), Row);
	Handle.RowName = TEXT("Prototype");
	const auto Prototype = MVSkillHUDPresentation::Build(Handle, Fallback);
	TestTrue(TEXT("Prototype is a selected icon with an explicit status"), Prototype.bPrototype && Prototype.Icons.Num() == 1 && !Prototype.Status.IsEmpty());
	const auto Empty = MVSkillHUDPresentation::Build({}, Fallback);
	TestTrue(TEXT("Empty and prototype remain distinct"), !Empty.bPrototype && Empty.Icons.IsEmpty() && !Empty.Status.EqualTo(Prototype.Status));
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVSkillHUDKeyHintTest, "Maverick.UI.SkillHUD.ActualInputHints",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVSkillHUDKeyHintTest::RunTest(const FString& Parameters)
{
	using MVSkillHUDPresentation::MakeKeyLabel;
	TestEqual(TEXT("Q remapping is reflected instead of a fixed Q label"), MakeKeyLabel({EKeys::F1}, {}, false, false).ToString(), FString(TEXT("F1")));
	TestEqual(TEXT("W uses modifier and mouse action"), MakeKeyLabel({EKeys::LeftMouseButton}, {EKeys::LeftShift}, true, false).ToString(), FString(TEXT("Shift+좌클릭")));
	TestEqual(TEXT("E modifier can also be remapped"), MakeKeyLabel({EKeys::RightMouseButton}, {EKeys::LeftControl}, true, false).ToString(), FString(TEXT("Ctrl+우클릭")));
	const auto Missing = MakeKeyLabel({}, {}, false, false);
	TestTrue(TEXT("Missing modifier never advertises unmodified attack"), MakeKeyLabel({EKeys::LeftMouseButton}, {}, true, false).EqualTo(Missing));
	TestTrue(TEXT("Gamepad never displays a keyboard-only binding"), MakeKeyLabel({EKeys::Q}, {}, false, true).EqualTo(Missing));
	TestFalse(TEXT("Mixed bindings choose gamepad key for gamepad"), MakeKeyLabel({EKeys::Q, EKeys::Gamepad_FaceButton_Bottom}, {}, false, true).EqualTo(Missing));
	TestTrue(TEXT("Unsupported touch cannot advertise keyboard controls"), MakeKeyLabel({EKeys::Q}, {}, false, false, true).EqualTo(Missing));
	UInputAction* Action = NewObject<UInputAction>();
	UInputAction* Modifier = NewObject<UInputAction>();
	UInputTriggerChordAction* Chord = NewObject<UInputTriggerChordAction>();
	Chord->ChordAction = Modifier;
	TArray<FEnhancedActionKeyMapping> Mappings;
	Mappings.Emplace(Action, EKeys::R);
	Mappings.Emplace(Action, EKeys::Gamepad_FaceButton_Top);
	Mappings.Last().Triggers.Add(Chord);
	Mappings.Emplace(Modifier, EKeys::Gamepad_LeftShoulder);
	TestEqual(TEXT("Keyboard mapping is not affected by the gamepad chord"), MVSkillHUDPresentation::MakeMappedKeyLabel(Mappings, Action, nullptr, false).ToString(), FString(TEXT("R")));
	TestEqual(TEXT("Gamepad R includes its Enhanced Input chord"), MVSkillHUDPresentation::MakeMappedKeyLabel(Mappings, Action, nullptr, true).ToString(), FString(TEXT("LB+위 버튼")));
	TestEqual(TEXT("Manual and authored same modifier is not repeated"), MVSkillHUDPresentation::MakeMappedKeyLabel(Mappings, Action, Modifier, true).ToString(), FString(TEXT("LB+위 버튼")));
	Mappings.Pop();
	TestTrue(TEXT("Unmapped chord cannot advertise an executable base key"), MVSkillHUDPresentation::MakeMappedKeyLabel(Mappings, Action, nullptr, true).EqualTo(Missing));
	Mappings.Emplace(Modifier, EKeys::Gamepad_LeftShoulder);
	UInputTriggerChordAction* Cycle = NewObject<UInputTriggerChordAction>();
	Cycle->ChordAction = Action;
	Mappings.Last().Triggers.Add(Cycle);
	TestTrue(TEXT("Cyclic input definitions terminate with unmapped indication"), MVSkillHUDPresentation::MakeMappedKeyLabel(Mappings, Action, nullptr, true).EqualTo(Missing));
	return !HasAnyErrors();
}
#endif
