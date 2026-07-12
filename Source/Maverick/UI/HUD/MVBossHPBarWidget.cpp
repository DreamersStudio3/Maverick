#include "UI/HUD/MVBossHPBarWidget.h"

#include "Components/MVStatComponent.h"
#include "Components/TextBlock.h"
#include "UI/HUD/MVStatusBarWidget.h"

void UMVBossHPBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (GroggyBar)
	{
		GroggyBar->SetBarFillType(EProgressBarFillType::FillFromCenterHorizontal);
	}
}

void UMVBossHPBarWidget::BindToStatComponent(UMVStatComponent* InStatComponent, FText BossName)
{
	if (BoundStatComponent == InStatComponent)
	{
		if (BossNameText)
		{
			BossNameText->SetText(BossName);
		}

		if (BoundStatComponent)
		{
			HandleHPChanged(BoundStatComponent->CurrentHP, BoundStatComponent->MaxHP);
			HandleGroggyChanged(BoundStatComponent->CurrentGroggy, BoundStatComponent->MaxGroggy);
			SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		return;
	}

	UnbindStatComponent();
	BoundStatComponent = InStatComponent;

	if (!BoundStatComponent)
	{
		ResetBossBar();
		return;
	}

	BoundStatComponent->OnHPChanged.AddUniqueDynamic(this, &UMVBossHPBarWidget::HandleHPChanged);
	BoundStatComponent->OnGroggyChanged.AddUniqueDynamic(this, &UMVBossHPBarWidget::HandleGroggyChanged);
	BoundStatComponent->OnGroggyStarted.AddUniqueDynamic(this, &UMVBossHPBarWidget::HandleGroggyStarted);
	BoundStatComponent->OnGroggyEnded.AddUniqueDynamic(this, &UMVBossHPBarWidget::HandleGroggyEnded);

	if (BossNameText)
	{
		BossNameText->SetText(BossName);
	}

	if (HPBar)
	{
		HPBar->SetProgressImmediate(BoundStatComponent->CurrentHP, BoundStatComponent->MaxHP);
	}
	if (GroggyBar)
	{
		GroggyBar->SetProgressImmediate(BoundStatComponent->CurrentGroggy, BoundStatComponent->MaxGroggy);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UMVBossHPBarWidget::UnbindStatComponent()
{
	if (!BoundStatComponent)
	{
		return;
	}

	BoundStatComponent->OnHPChanged.RemoveDynamic(this, &UMVBossHPBarWidget::HandleHPChanged);
	BoundStatComponent->OnGroggyChanged.RemoveDynamic(this, &UMVBossHPBarWidget::HandleGroggyChanged);
	BoundStatComponent->OnGroggyStarted.RemoveDynamic(this, &UMVBossHPBarWidget::HandleGroggyStarted);
	BoundStatComponent->OnGroggyEnded.RemoveDynamic(this, &UMVBossHPBarWidget::HandleGroggyEnded);
	BoundStatComponent = nullptr;
}

void UMVBossHPBarWidget::InitializeBossBar(FText BossName, float MaxHP)
{
	UnbindStatComponent();

	if (BossNameText)
	{
		BossNameText->SetText(BossName);
	}

	if (HPBar)
	{
		HPBar->SetProgressImmediate(MaxHP, MaxHP);
	}
	if (GroggyBar)
	{
		GroggyBar->SetProgressImmediate(0.0f, 1.0f);
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
	UnbindStatComponent();

	if (BossNameText)
	{
		BossNameText->SetText(FText::GetEmpty());
	}

	if (HPBar)
	{
		HPBar->SetProgressImmediate(0.0f, 1.0f);
	}
	if (GroggyBar)
	{
		GroggyBar->SetProgressImmediate(0.0f, 1.0f);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UMVBossHPBarWidget::NativeDestruct()
{
	UnbindStatComponent();
	Super::NativeDestruct();
}

void UMVBossHPBarWidget::HandleHPChanged(float CurrentHP, float MaxHP)
{
	UpdateBossHP(CurrentHP, MaxHP);
}

void UMVBossHPBarWidget::HandleGroggyChanged(float CurrentGroggy, float MaxGroggy)
{
	if (GroggyBar)
	{
		GroggyBar->SetProgress(CurrentGroggy, MaxGroggy);
	}
}

void UMVBossHPBarWidget::HandleGroggyStarted()
{
	if (GroggyBar && BoundStatComponent)
	{
		GroggyBar->SetProgressImmediate(BoundStatComponent->CurrentGroggy, BoundStatComponent->MaxGroggy);
	}
}

void UMVBossHPBarWidget::HandleGroggyEnded()
{
	if (GroggyBar && BoundStatComponent)
	{
		GroggyBar->SetProgress(BoundStatComponent->CurrentGroggy, BoundStatComponent->MaxGroggy);
	}
}
