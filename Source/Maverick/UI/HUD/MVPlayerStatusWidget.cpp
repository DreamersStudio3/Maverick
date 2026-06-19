#include "UI/HUD/MVPlayerStatusWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/Pawn.h"
#include "UI/HUD/MVStatusBarWidget.h"

void UMVPlayerStatusWidget::BindToStatComponent(UMVStatComponent* InStatComponent)
{
	if (BoundStatComponent == InStatComponent)
	{
		return;
	}

	UnbindStatComponent();
	BoundStatComponent = InStatComponent;

	if (!BoundStatComponent)
	{
		return;
	}

	BoundStatComponent->OnHPChanged.AddUniqueDynamic(this, &UMVPlayerStatusWidget::HandleHPChanged);
	BoundStatComponent->OnStaminaChanged.AddUniqueDynamic(this, &UMVPlayerStatusWidget::HandleStaminaChanged);
	BoundStatComponent->OnMPChanged.AddUniqueDynamic(this, &UMVPlayerStatusWidget::HandleMPChanged);

	HandleHPChanged(BoundStatComponent->CurrentHP, BoundStatComponent->MaxHP);
	HandleStaminaChanged(BoundStatComponent->CurrentStamina, BoundStatComponent->MaxStamina);
	HandleMPChanged(BoundStatComponent->CurrentMP, BoundStatComponent->MaxMP);
}

void UMVPlayerStatusWidget::UnbindStatComponent()
{
	if (!BoundStatComponent)
	{
		return;
	}

	BoundStatComponent->OnHPChanged.RemoveDynamic(this, &UMVPlayerStatusWidget::HandleHPChanged);
	BoundStatComponent->OnStaminaChanged.RemoveDynamic(this, &UMVPlayerStatusWidget::HandleStaminaChanged);
	BoundStatComponent->OnMPChanged.RemoveDynamic(this, &UMVPlayerStatusWidget::HandleMPChanged);
	BoundStatComponent = nullptr;
}

void UMVPlayerStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildNativeWidgetTree();
	ConfigureStatusBars();
}

void UMVPlayerStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ConfigureStatusBars();

	APawn* OwningPawn = GetOwningPlayerPawn();
	BindToStatComponent(OwningPawn ? OwningPawn->FindComponentByClass<UMVStatComponent>() : nullptr);
	BindToCharacter(Cast<AMVCharacterBase>(OwningPawn));
}

void UMVPlayerStatusWidget::NativeDestruct()
{
	UnbindCharacter();
	UnbindStatComponent();
	Super::NativeDestruct();
}

void UMVPlayerStatusWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PlayerStatusRoot"));
	HPBar = WidgetTree->ConstructWidget<UMVStatusBarWidget>(UMVStatusBarWidget::StaticClass(), TEXT("HPBar"));
	StaminaBar = WidgetTree->ConstructWidget<UMVStatusBarWidget>(UMVStatusBarWidget::StaticClass(), TEXT("StaminaBar"));
	MPBar = WidgetTree->ConstructWidget<UMVStatusBarWidget>(UMVStatusBarWidget::StaticClass(), TEXT("MPBar"));

	WidgetTree->RootWidget = RootBox;

	if (UVerticalBoxSlot* HPSlot = RootBox->AddChildToVerticalBox(HPBar))
	{
		FSlateChildSize SlotSize;
		SlotSize.SizeRule = ESlateSizeRule::Fill;
		SlotSize.Value = 1.5f;
		HPSlot->SetSize(SlotSize);
		HPSlot->SetHorizontalAlignment(HAlign_Fill);
		HPSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UVerticalBoxSlot* MPSlot = RootBox->AddChildToVerticalBox(MPBar))
	{
		FSlateChildSize SlotSize;
		SlotSize.SizeRule = ESlateSizeRule::Fill;
		SlotSize.Value = 1.0f;
		MPSlot->SetSize(SlotSize);
		MPSlot->SetHorizontalAlignment(HAlign_Fill);
		MPSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UVerticalBoxSlot* StaminaSlot = RootBox->AddChildToVerticalBox(StaminaBar))
	{
		FSlateChildSize SlotSize;
		SlotSize.SizeRule = ESlateSizeRule::Fill;
		SlotSize.Value = 1.0f;
		StaminaSlot->SetSize(SlotSize);
		StaminaSlot->SetHorizontalAlignment(HAlign_Fill);
		StaminaSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UMVPlayerStatusWidget::ConfigureStatusBars()
{
	if (HPBar)
	{
		HPBar->SetFillColor(FLinearColor(0.82f, 0.08f, 0.08f, 1.0f));
		ApplyStatusBarSize(HPBar, StatusBarReferenceMaxValue, HPBaseBarWidth, HPMaxBarWidth, HPWidthPerMaxPoint, HPBarHeight);
		HPBar->SetLabelVisible(false);
		HPBar->SetValueVisible(false);
	}

	if (StaminaBar)
	{
		StaminaBar->SetFillColor(FLinearColor(0.08f, 0.62f, 0.22f, 1.0f));
		ApplyStatusBarSize(StaminaBar, StatusBarReferenceMaxValue, StaminaBaseBarWidth, StaminaMaxBarWidth, StaminaWidthPerMaxPoint, StaminaBarHeight);
		StaminaBar->SetLabelVisible(false);
		StaminaBar->SetValueVisible(false);
	}

	if (MPBar)
	{
		MPBar->SetFillColor(FLinearColor(0.13f, 0.32f, 0.88f, 1.0f));
		ApplyStatusBarSize(MPBar, StatusBarReferenceMaxValue, MPBaseBarWidth, MPMaxBarWidth, MPWidthPerMaxPoint, MPBarHeight);
		MPBar->SetLabelVisible(false);
		MPBar->SetValueVisible(false);
	}
}

void UMVPlayerStatusWidget::BindToCharacter(AMVCharacterBase* InCharacter)
{
	if (BoundCharacter == InCharacter)
	{
		return;
	}

	UnbindCharacter();
	BoundCharacter = InCharacter;
	if (!BoundCharacter)
	{
		return;
	}

	BoundCharacter->OnStatRecentLossHoldChanged.AddUniqueDynamic(
		this,
		&UMVPlayerStatusWidget::HandleStatRecentLossHoldChanged);
}

void UMVPlayerStatusWidget::UnbindCharacter()
{
	if (!BoundCharacter)
	{
		return;
	}

	BoundCharacter->OnStatRecentLossHoldChanged.RemoveDynamic(
		this,
		&UMVPlayerStatusWidget::HandleStatRecentLossHoldChanged);
	BoundCharacter = nullptr;
}

void UMVPlayerStatusWidget::ApplyStatusBarSize(
	UMVStatusBarWidget* StatusBar,
	float MaxValue,
	float BaseWidth,
	float MaxWidth,
	float WidthPerMaxPoint,
	float Height) const
{
	if (!StatusBar)
	{
		return;
	}

	const float ExtraMaxValue = FMath::Max(0.0f, MaxValue - StatusBarReferenceMaxValue);
	const float TargetWidth = FMath::Clamp(BaseWidth + ExtraMaxValue * WidthPerMaxPoint, 0.0f, MaxWidth);
	StatusBar->SetBarSize(TargetWidth, Height);
}

void UMVPlayerStatusWidget::HandleHPChanged(float CurrentValue, float MaxValue)
{
	if (HPBar)
	{
		ApplyStatusBarSize(HPBar, MaxValue, HPBaseBarWidth, HPMaxBarWidth, HPWidthPerMaxPoint, HPBarHeight);
		HPBar->SetProgress(CurrentValue, MaxValue);
	}
}

void UMVPlayerStatusWidget::HandleStaminaChanged(float CurrentValue, float MaxValue)
{
	if (StaminaBar)
	{
		ApplyStatusBarSize(StaminaBar, MaxValue, StaminaBaseBarWidth, StaminaMaxBarWidth, StaminaWidthPerMaxPoint, StaminaBarHeight);
		StaminaBar->SetProgress(CurrentValue, MaxValue);
	}
}

void UMVPlayerStatusWidget::HandleMPChanged(float CurrentValue, float MaxValue)
{
	if (MPBar)
	{
		ApplyStatusBarSize(MPBar, MaxValue, MPBaseBarWidth, MPMaxBarWidth, MPWidthPerMaxPoint, MPBarHeight);
		MPBar->SetProgress(CurrentValue, MaxValue);
	}
}

void UMVPlayerStatusWidget::HandleStatRecentLossHoldChanged(bool bInHold)
{
	if (HPBar)
	{
		HPBar->SetRecentLossHoldLocked(bInHold);
	}

	if (StaminaBar)
	{
		StaminaBar->SetRecentLossHoldLocked(bInHold);
	}

	if (MPBar)
	{
		MPBar->SetRecentLossHoldLocked(bInHold);
	}
}
