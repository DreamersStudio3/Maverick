#include "UI/World/MVMonsterHPBarWidget.h"

#include "Components/MVStatComponent.h"
#include "UI/HUD/MVStatusBarWidget.h"

void UMVMonsterHPBarWidget::BindToActor(AActor* TargetActor)
{
	UMVStatComponent* NewStatComponent = TargetActor ? TargetActor->FindComponentByClass<UMVStatComponent>() : nullptr;
	if (BoundStatComponent == NewStatComponent)
	{
		return;
	}

	UnbindStatComponent();
	BoundStatComponent = NewStatComponent;

	if (!BoundStatComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	BoundStatComponent->OnHPChanged.AddUniqueDynamic(this, &UMVMonsterHPBarWidget::HandleHPChanged);
	HandleHPChanged(BoundStatComponent->CurrentHP, BoundStatComponent->MaxHP);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UMVMonsterHPBarWidget::UnbindStatComponent()
{
	if (!BoundStatComponent)
	{
		return;
	}

	BoundStatComponent->OnHPChanged.RemoveDynamic(this, &UMVMonsterHPBarWidget::HandleHPChanged);
	BoundStatComponent = nullptr;
}

void UMVMonsterHPBarWidget::NativeDestruct()
{
	UnbindStatComponent();
	Super::NativeDestruct();
}

void UMVMonsterHPBarWidget::HandleHPChanged(float CurrentValue, float MaxValue)
{
	if (HPBar)
	{
		HPBar->SetProgress(CurrentValue, MaxValue);
	}
}
