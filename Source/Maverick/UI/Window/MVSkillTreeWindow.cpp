#include "UI/Window/MVSkillTreeWindow.h"

#include "UI/Window/MVSkillNodeButton.h"

void UMVSkillTreeWindow::NativeOnActivated()
{
	Super::NativeOnActivated();
	RequestRebuildExplicitNavigation();
}

void UMVSkillTreeWindow::RequestRebuildExplicitNavigation()
{
	BP_RebuildExplicitNavigation();
}

UWidget* UMVSkillTreeWindow::NativeGetDesiredFocusTarget() const
{
	if (InitialSkillNode)
	{
		return InitialSkillNode;
	}

	return Super::NativeGetDesiredFocusTarget();
}
