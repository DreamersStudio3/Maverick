#include "UI/System/MVUINavigationLibrary.h"

#include "Blueprint/WidgetNavigation.h"
#include "Components/Widget.h"

void UMVUINavigationLibrary::SetExplicitNavigation(UWidget* FromWidget, EUINavigation Direction, UWidget* ToWidget)
{
	if (!FromWidget || !ToWidget)
	{
		return;
	}

	FromWidget->SetNavigationRuleExplicit(Direction, ToWidget);
}

void UMVUINavigationLibrary::SetNavigationStop(UWidget* FromWidget, EUINavigation Direction)
{
	if (!FromWidget)
	{
		return;
	}

	FromWidget->SetNavigationRuleBase(Direction, EUINavigationRule::Stop);
}

void UMVUINavigationLibrary::SetCompassNavigation(
	UWidget* FromWidget,
	UWidget* UpWidget,
	UWidget* DownWidget,
	UWidget* LeftWidget,
	UWidget* RightWidget)
{
	SetExplicitNavigation(FromWidget, EUINavigation::Up, UpWidget);
	SetExplicitNavigation(FromWidget, EUINavigation::Down, DownWidget);
	SetExplicitNavigation(FromWidget, EUINavigation::Left, LeftWidget);
	SetExplicitNavigation(FromWidget, EUINavigation::Right, RightWidget);
}
