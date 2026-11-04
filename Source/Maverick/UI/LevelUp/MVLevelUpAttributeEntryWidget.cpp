#include "UI/LevelUp/MVLevelUpAttributeEntryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/LevelUp/MVLevelUpStatNameWidget.h"
#include "CommonInputSubsystem.h"
#include "Components/Border.h"

UMVLevelUpAttributeEntryWidget::UMVLevelUpAttributeEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoFadeInOnConstruct = false;
}

void UMVLevelUpAttributeEntryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsDesignTime())
	{
		ApplyDisplay();
	}
}

void UMVLevelUpAttributeEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonDecrease)
	{
		ButtonDecrease->OnClicked.AddUniqueDynamic(
			this,
			&UMVLevelUpAttributeEntryWidget::HandleDecreaseClicked);
	}

	if (ButtonIncrease)
	{
		ButtonIncrease->OnClicked.AddUniqueDynamic(
			this,
			&UMVLevelUpAttributeEntryWidget::HandleIncreaseClicked);
	}

	ApplyDisplay();
	ApplySelection();
}

void UMVLevelUpAttributeEntryWidget::NativeDestruct()
{
	if (ButtonDecrease)
	{
		ButtonDecrease->OnClicked.RemoveDynamic(
			this,
			&UMVLevelUpAttributeEntryWidget::HandleDecreaseClicked);
	}

	if (ButtonIncrease)
	{
		ButtonIncrease->OnClicked.RemoveDynamic(
			this,
			&UMVLevelUpAttributeEntryWidget::HandleIncreaseClicked);
	}

	Super::NativeDestruct();
}

void UMVLevelUpAttributeEntryWidget::SetEntry(
	const FMVProgressionEntryDefinition& Entry,
	const int32 InCurrentRank,
	const int32 InPreviewRank,
	const bool bInCanDecrease,
	const bool bInCanIncrease)
{
	ProgressionId = Entry.ProgressionId;
	CachedDisplayName = Entry.DisplayName;

	CurrentRank = InCurrentRank;
	PreviewRank = InPreviewRank;

	bCanDecrease = bInCanDecrease;
	bCanIncrease = bInCanIncrease;
	bHasEntry = true;

	SetToolTipText(Entry.Description);
	ApplyDisplay();
}

void UMVLevelUpAttributeEntryWidget::ApplyDisplay()
{
	if (bHasEntry)
	{
		if (StatNameWidget)
		{
			StatNameWidget->SetStatName(CachedDisplayName);
		}

		if (StatValueOld)
		{
			StatValueOld->SetText(FText::AsNumber(CurrentRank));
		}

		if (StatValueNew)
		{
			StatValueNew->SetText(FText::AsNumber(PreviewRank));
		}
	}

	if (ButtonDecrease)
	{
		ButtonDecrease->SetIsEnabled(bHasEntry && bCanDecrease);
	}

	if (ButtonIncrease)
	{
		ButtonIncrease->SetIsEnabled(bHasEntry && bCanIncrease);
	}
}

void UMVLevelUpAttributeEntryWidget::HandleDecreaseClicked()
{
	if (bHasEntry && bCanDecrease && ProgressionId.IsValid())
	{
		OnRankAdjustmentRequested.Broadcast(
			ProgressionId,
			-1);
	}
}

void UMVLevelUpAttributeEntryWidget::HandleIncreaseClicked()
{
	if (bHasEntry && bCanIncrease && ProgressionId.IsValid())
	{
		OnRankAdjustmentRequested.Broadcast(
			ProgressionId,
			1);
	}
}

void UMVLevelUpAttributeEntryWidget::SetSelected(const bool bInSelected)
{
	bSelected = bInSelected;
	ApplySelection();
}

void UMVLevelUpAttributeEntryWidget::ApplySelection()
{
	if (SelectionBorder)
	{
		SelectionBorder->SetBrushColor(
			bSelected ? SelectedBackgroundColor : UnselectedBackgroundColor);
	}
}

void UMVLevelUpAttributeEntryWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());

	if (InputSubsystem
		&& InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		return;
	}

	if (bHasEntry && ProgressionId.IsValid())
	{
		OnSelectionRequested.Broadcast(ProgressionId);
	}
}