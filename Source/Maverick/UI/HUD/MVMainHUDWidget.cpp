#include "UI/HUD/MVMainHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/Pawn.h"
#include "Character/PC/Consumable/MVPlayerConsumableComponent.h"
#include "Components/MVStatComponent.h"
#include "UI/HUD/MVBossHPBarWidget.h"
#include "UI/HUD/MVPlayerStatusWidget.h"
#include "UI/HUD/MVQuickSlotWidget.h"

void UMVMainHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildNativeWidgetTree();
}

void UMVMainHUDWidget::RefreshHUD()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (PlayerStatus)
	{
		PlayerStatus->BindToStatComponent(OwningPawn ? OwningPawn->FindComponentByClass<UMVStatComponent>() : nullptr);
	}

	BindPlayerConsumableComponent(
		OwningPawn ? OwningPawn->FindComponentByClass<UMVPlayerConsumableComponent>() : nullptr);
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
	HPSlot = WidgetTree->ConstructWidget<UMVQuickSlotWidget>(UMVQuickSlotWidget::StaticClass(), TEXT("HPSlot"));

	WidgetTree->RootWidget = RootCanvas;

	if (UCanvasPanelSlot* PlayerStatusSlot = RootCanvas->AddChildToCanvas(PlayerStatus))
	{
		PlayerStatusSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		PlayerStatusSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		PlayerStatusSlot->SetAutoSize(true);
		PlayerStatusSlot->SetPosition(FVector2D(32.0f, 32.0f));
	}

	if (UCanvasPanelSlot* HPSlotCanvasSlot = RootCanvas->AddChildToCanvas(HPSlot))
	{
		HPSlotCanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f));
		HPSlotCanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
		HPSlotCanvasSlot->SetAutoSize(true);
		HPSlotCanvasSlot->SetPosition(FVector2D(32.0f, -32.0f));
	}
}

void UMVMainHUDWidget::BindPlayerConsumableComponent(UMVPlayerConsumableComponent* ConsumableComponent)
{
	if (BoundPlayerConsumableComponent == ConsumableComponent)
	{
		ApplyHealingPotionQuickSlotView();
		return;
	}

	if (BoundPlayerConsumableComponent)
	{
		BoundPlayerConsumableComponent->OnHealingPotionStateChanged.RemoveDynamic(
			this,
			&UMVMainHUDWidget::HandleHealingPotionStateChanged);
	}

	BoundPlayerConsumableComponent = ConsumableComponent;

	if (BoundPlayerConsumableComponent)
	{
		BoundPlayerConsumableComponent->OnHealingPotionStateChanged.AddUniqueDynamic(
			this,
			&UMVMainHUDWidget::HandleHealingPotionStateChanged);
	}

	ApplyHealingPotionQuickSlotView();
}

void UMVMainHUDWidget::ApplyHealingPotionQuickSlotView()
{
	if (!HPSlot)
	{
		return;
	}

	if (!BoundPlayerConsumableComponent)
	{
		FMVQuickSlotViewData EmptyViewData;
		EmptyViewData.bLocked = true;
		HPSlot->SetViewData(EmptyViewData);
		return;
	}

	HPSlot->SetViewData(BoundPlayerConsumableComponent->BuildHealingPotionQuickSlotViewData());
}

void UMVMainHUDWidget::HandleHealingPotionStateChanged(
	const FMVHealingPotionRuntimeState& /*HealingPotionState*/)
{
	ApplyHealingPotionQuickSlotView();
}
