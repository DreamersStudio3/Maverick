#include "UI/HUD/MVPlayerStatusWidget.h"

#include "Components/MVStatComponent.h"
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

void UMVPlayerStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();
	BindToStatComponent(OwningPawn ? OwningPawn->FindComponentByClass<UMVStatComponent>() : nullptr);
}

void UMVPlayerStatusWidget::NativeDestruct()
{
	UnbindStatComponent();
	Super::NativeDestruct();
}

void UMVPlayerStatusWidget::HandleHPChanged(float CurrentValue, float MaxValue)
{
	if (HPBar)
	{
		HPBar->SetProgress(CurrentValue, MaxValue);
	}
}

void UMVPlayerStatusWidget::HandleStaminaChanged(float CurrentValue, float MaxValue)
{
	if (StaminaBar)
	{
		StaminaBar->SetProgress(CurrentValue, MaxValue);
	}
}

void UMVPlayerStatusWidget::HandleMPChanged(float CurrentValue, float MaxValue)
{
	if (MPBar)
	{
		MPBar->SetProgress(CurrentValue, MaxValue);
	}
}
