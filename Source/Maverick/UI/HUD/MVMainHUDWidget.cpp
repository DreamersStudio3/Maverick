#include "UI/HUD/MVMainHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/Pawn.h"
#include "Components/MVStatComponent.h"
#include "UI/HUD/MVBossHPBarWidget.h"
#include "UI/HUD/MVPlayerStatusWidget.h"

void UMVMainHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildNativeWidgetTree();
}

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

void UMVMainHUDWidget::BindBossStatus(UMVStatComponent* BossStatComponent, FText BossName)
{
	CachedBossName = BossName;

	if (BossHPBar)
	{
		BossHPBar->BindToStatComponent(BossStatComponent, BossName);
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

void UMVMainHUDWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUDRoot"));
	PlayerStatus = WidgetTree->ConstructWidget<UMVPlayerStatusWidget>(UMVPlayerStatusWidget::StaticClass(), TEXT("PlayerStatus"));

	WidgetTree->RootWidget = RootCanvas;

	if (UCanvasPanelSlot* PlayerStatusSlot = RootCanvas->AddChildToCanvas(PlayerStatus))
	{
		PlayerStatusSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		PlayerStatusSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		PlayerStatusSlot->SetAutoSize(true);
		PlayerStatusSlot->SetPosition(FVector2D(32.0f, 32.0f));
	}
}
