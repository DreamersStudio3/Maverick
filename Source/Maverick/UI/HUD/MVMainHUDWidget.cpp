#include "UI/HUD/MVMainHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/Pawn.h"
#include "Character/PC/Consumable/MVPlayerConsumable.h"
#include "Components/MVCombatComponent.h"
#include "Components/MVStatComponent.h"
#include "UI/HUD/MVBossHPBarWidget.h"
#include "UI/HUD/MVPlayerSkillHUDWidget.h"
#include "UI/HUD/MVPlayerStatusWidget.h"
#include "UI/HUD/MVQuickSlotWidget.h"

void UMVMainHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildNativeWidgetTree();
	EnsurePlayerSkillHUD();
}

void UMVMainHUDWidget::RefreshHUD()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (PlayerStatus)
	{
		PlayerStatus->BindToStatComponent(OwningPawn ? OwningPawn->FindComponentByClass<UMVStatComponent>() : nullptr);
	}
	if (PlayerSkillHUD)
	{
		PlayerSkillHUD->BindToCombatComponent(
			OwningPawn ? OwningPawn->FindComponentByClass<UMVCombatComponent>() : nullptr);
	}

	AMVPlayerCharacter* PlayerCharacter = Cast<AMVPlayerCharacter>(OwningPawn);
	BindPlayerConsumable(PlayerCharacter ? PlayerCharacter->PlayerConsumable : nullptr);
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

	const APawn* OwningPawn = GetOwningPlayerPawn();
	if (BossStatComponent && BossStatComponent->GetOwner() == OwningPawn)
	{
		return;
	}

	if (BossHPBar)
	{
		BossHPBar->BindToStatComponent(BossStatComponent, BossName);
	}
}

void UMVMainHUDWidget::UpdateBossStatus(float CurrentHP, float MaxHP)
{
	if (BossHPBar && BossHPBar->IsBoundToStatComponent())
	{
		return;
	}

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
	PlayerSkillHUD = WidgetTree->ConstructWidget<UMVPlayerSkillHUDWidget>(
		UMVPlayerSkillHUDWidget::StaticClass(),
		TEXT("PlayerSkillHUD"));

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

	if (UCanvasPanelSlot* SkillHUDSlot = RootCanvas->AddChildToCanvas(PlayerSkillHUD))
	{
		SkillHUDSlot->SetAnchors(FAnchors(0.5f, 1.0f));
		SkillHUDSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		SkillHUDSlot->SetAutoSize(true);
		SkillHUDSlot->SetPosition(PlayerSkillHUDCanvasOffset);
	}
}

void UMVMainHUDWidget::EnsurePlayerSkillHUD()
{
	if (PlayerSkillHUD || !WidgetTree)
	{
		return;
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return;
	}

	PlayerSkillHUD = WidgetTree->ConstructWidget<UMVPlayerSkillHUDWidget>(
		UMVPlayerSkillHUDWidget::StaticClass(),
		TEXT("PlayerSkillHUD"));
	if (UCanvasPanelSlot* SkillHUDSlot = RootCanvas->AddChildToCanvas(PlayerSkillHUD))
	{
		SkillHUDSlot->SetAnchors(FAnchors(0.5f, 1.0f));
		SkillHUDSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		SkillHUDSlot->SetAutoSize(true);
		SkillHUDSlot->SetPosition(PlayerSkillHUDCanvasOffset);
	}
}

void UMVMainHUDWidget::BindPlayerConsumable(UMVPlayerConsumable* Consumable)
{
	if (BoundPlayerConsumable == Consumable)
	{
		ApplyHealingPotionQuickSlotView();
		return;
	}

	if (BoundPlayerConsumable)
	{
		BoundPlayerConsumable->OnHealingPotionStateChanged.RemoveDynamic(
			this,
			&UMVMainHUDWidget::HandleHealingPotionStateChanged);
	}

	BoundPlayerConsumable = Consumable;

	if (BoundPlayerConsumable)
	{
		BoundPlayerConsumable->OnHealingPotionStateChanged.AddUniqueDynamic(
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

	if (!BoundPlayerConsumable)
	{
		FMVQuickSlotViewData EmptyViewData;
		EmptyViewData.bLocked = true;
		HPSlot->SetViewData(EmptyViewData);
		return;
	}

	HPSlot->SetViewData(BoundPlayerConsumable->BuildHealingPotionQuickSlotViewData());
}

void UMVMainHUDWidget::HandleHealingPotionStateChanged(
	const FMVHealingPotionRuntimeState& /*HealingPotionState*/)
{
	ApplyHealingPotionQuickSlotView();
}
