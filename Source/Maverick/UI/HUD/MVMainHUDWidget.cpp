#include "UI/HUD/MVMainHUDWidget.h"

#include "GameFramework/Pawn.h"
#include "Components/MVStatComponent.h"
#include "UI/HUD/MVBossHPBarWidget.h"
#include "UI/HUD/MVPlayerStatusWidget.h"

void UMVMainHUDWidget::RefreshHUD()
{
	if (!PlayerStatus)
	{
		return;
	}

	APawn* OwningPawn = GetOwningPlayerPawn();
	PlayerStatus->BindToStatComponent(OwningPawn ? OwningPawn->FindComponentByClass<UMVStatComponent>() : nullptr);
}

void UMVMainHUDWidget::InitBossStatus(FText BossName, float MaxHP)
{
	CachedBossName = BossName;

	if (BossHPBar)
	{
		BossHPBar->InitializeBossBar(BossName, MaxHP);
	}
}

void UMVMainHUDWidget::UpdateBossStatus(float CurrentHP, float MaxHP)
{
	if (BossHPBar)
	{
		BossHPBar->UpdateBossHP(CurrentHP, MaxHP);
	}
}

void UMVMainHUDWidget::HideBossHPBar()
{
	if (BossHPBar)
	{
		BossHPBar->ResetBossBar();
	}
}
