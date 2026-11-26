#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Input/CommonUIInputTypes.h"
#include "UI/System/MVGameplayInputWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVMenuInputGuardTest, "Maverick.UI.MenuInputGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVMenuInputGuardTest::RunTest(const FString& Parameters)
{
	UMVGameplayInputWidget* Input = NewObject<UMVGameplayInputWidget>();
	TestEqual(TEXT("An empty layer permits gameplay"), Input->GetDesiredInputConfig()->GetInputMode(), ECommonInputMode::Game);
	for (int32 Transition = 0; Transition < 3; ++Transition)
	{
		Input->SetMenuInputBlocked(true);
		const FUIInputConfig Menu = Input->GetDesiredInputConfig().GetValue();
		TestEqual(TEXT("Menu fallback consumes gameplay input throughout screen changes"), Menu.GetInputMode(), ECommonInputMode::Menu);
		TestTrue(TEXT("Menu fallback blocks movement and camera"), Menu.bIgnoreMoveInput && Menu.bIgnoreLookInput);
		TestEqual(TEXT("Menu fallback releases mouse capture"), Menu.GetMouseCaptureMode(), EMouseCaptureMode::NoCapture);
		TestFalse(TEXT("Menu fallback keeps cursor visible during capture"), Menu.HideCursorDuringViewportCapture());
	}
	Input->SetMenuInputBlocked(false);
	const FUIInputConfig Game = Input->GetDesiredInputConfig().GetValue();
	TestEqual(TEXT("Final menu close restores gameplay"), Game.GetInputMode(), ECommonInputMode::Game);
	TestFalse(TEXT("Final menu close restores movement and camera"), Game.bIgnoreMoveInput || Game.bIgnoreLookInput);
	return !HasAnyErrors();
}
#endif
