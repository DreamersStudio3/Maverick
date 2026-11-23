#include "UI/System/MVUISettings.h"

#include "InputAction.h"
#include "UI/HUD/MVMainHUDWidget.h"
#include "UI/Popup/MVBossClearPopup.h"
#include "UI/Popup/MVInteractionPromptPopup.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/Window/MVDeathOverlayWindow.h"
#include "UI/Window/MVDialogueWindow.h"
#include "UI/Window/MVEscapeWindow.h"
#include "UI/Window/MVLoadingWindow.h"
#include "UI/Window/MVSkillTreeWindow.h"
#include "UI/Window/MVTitleWindow.h"
#include "UObject/ConstructorHelpers.h"

UMVUISettings::UMVUISettings()
{
	static const ConstructorHelpers::FObjectFinder<UInputAction> MenuAction(
		TEXT("/Game/Input/Actions/IA_InventoryMenu.IA_InventoryMenu"));
	InventoryMenuAction = MenuAction.Object;

	RootLayerClass = UMVUILayerBase::StaticClass();
	DefaultHUDClass = UMVMainHUDWidget::StaticClass();
	TitleWindowClass = UMVTitleWindow::StaticClass();
	EscapeWindowClass = UMVEscapeWindow::StaticClass();
	LoadingWindowClass = UMVLoadingWindow::StaticClass();
	DeathOverlayWindowClass = UMVDeathOverlayWindow::StaticClass();
	SkillTreeWindowClass = UMVSkillTreeWindow::StaticClass();
	InteractionPromptPopupClass = UMVInteractionPromptPopup::StaticClass();
	BossClearPopupClass = UMVBossClearPopup::StaticClass();
	DialogueWindowClass = UMVDialogueWindow::StaticClass();
	MessagePopupClass = UMVMessagePopup::StaticClass();
}
