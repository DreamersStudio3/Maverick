#include "UI/Window/MVSkillNodeButton.h"

void UMVSkillNodeButton::NativeConstruct()
{
	Super::NativeConstruct();
	ApplyExplicitNavigation();
}

void UMVSkillNodeButton::NativeOnClicked()
{
	Super::NativeOnClicked();
	OnSkillNodeClicked.Broadcast(SkillId);
}

void UMVSkillNodeButton::SetExplicitSkillNavigation(
	UMVSkillNodeButton* UpNode,
	UMVSkillNodeButton* DownNode,
	UMVSkillNodeButton* LeftNode,
	UMVSkillNodeButton* RightNode)
{
	ExplicitUpNode = UpNode;
	ExplicitDownNode = DownNode;
	ExplicitLeftNode = LeftNode;
	ExplicitRightNode = RightNode;
	ApplyExplicitNavigation();
}

void UMVSkillNodeButton::ApplyExplicitNavigation()
{
	if (ExplicitUpNode)
	{
		SetNavigationRuleExplicit(EUINavigation::Up, ExplicitUpNode);
	}

	if (ExplicitDownNode)
	{
		SetNavigationRuleExplicit(EUINavigation::Down, ExplicitDownNode);
	}

	if (ExplicitLeftNode)
	{
		SetNavigationRuleExplicit(EUINavigation::Left, ExplicitLeftNode);
	}

	if (ExplicitRightNode)
	{
		SetNavigationRuleExplicit(EUINavigation::Right, ExplicitRightNode);
	}
}
