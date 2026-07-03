#include "UI/System/MVUISettings.h"

#include "UI/HUD/MVMainHUDWidget.h"
#include "UI/Popup/MVBossClearPopup.h"
#include "UI/Popup/MVDialoguePopup.h"
#include "UI/Popup/MVInteractionPromptPopup.h"
#include "UI/Popup/MVMessagePopup.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/Window/MVDeathOverlayWindow.h"
#include "UI/Window/MVEscapeWindow.h"
#include "UI/Window/MVLoadingWindow.h"
#include "UI/Window/MVSkillTreeWindow.h"
#include "UI/Window/MVTitleWindow.h"

UMVUISettings::UMVUISettings()
{
	RootLayerClass = UMVUILayerBase::StaticClass();
	DefaultHUDClass = UMVMainHUDWidget::StaticClass();
	TitleWindowClass = UMVTitleWindow::StaticClass();
	EscapeWindowClass = UMVEscapeWindow::StaticClass();
	LoadingWindowClass = UMVLoadingWindow::StaticClass();
	DeathOverlayWindowClass = UMVDeathOverlayWindow::StaticClass();
	SkillTreeWindowClass = UMVSkillTreeWindow::StaticClass();
	InteractionPromptPopupClass = UMVInteractionPromptPopup::StaticClass();
	BossClearPopupClass = UMVBossClearPopup::StaticClass();
	DialoguePopupClass = UMVDialoguePopup::StaticClass();
	MessagePopupClass = UMVMessagePopup::StaticClass();
}
