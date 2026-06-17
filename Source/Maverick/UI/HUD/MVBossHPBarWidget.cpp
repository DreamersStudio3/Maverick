#include "UI/HUD/MVBossHPBarWidget.h"

#include "Components/TextBlock.h"
#include "UI/HUD/MVStatusBarWidget.h"

void UMVBossHPBarWidget::InitializeBossBar(FText BossName, float MaxHP)
{
	if (BossNameText)
	{
		BossNameText->SetText(BossName);
	}

	if (HPBar)
	{
		HPBar->SetProgressImmediate(MaxHP, MaxHP);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UMVBossHPBarWidget::UpdateBossHP(float CurrentHP, float MaxHP)
{
	if (HPBar)
	{
		HPBar->SetProgress(CurrentHP, MaxHP);
	}
}

void UMVBossHPBarWidget::ResetBossBar()
{
	if (BossNameText)
	{
		BossNameText->SetText(FText::GetEmpty());
	}

	if (HPBar)
	{
		HPBar->SetProgressImmediate(0.0f, 1.0f);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}
