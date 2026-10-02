#include "UI/LevelUp/MVLevelUpStatBlockWidget.h"

#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "UI/LevelUp/MVLevelUpAttributeEntryWidget.h"
#include "UI/LevelUp/MVLevelUpStatComparisonWidget.h"

UMVLevelUpStatBlockWidget::UMVLevelUpStatBlockWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoFadeInOnConstruct = false;
}

void UMVLevelUpStatBlockWidget::ClearEntries()
{
	if (!IsDesignTime() && StatBox)
	{
		StatBox->ClearChildren();
	}
}

UMVLevelUpAttributeEntryWidget* UMVLevelUpStatBlockWidget::AddAttributeEntry(
	TSubclassOf<UMVLevelUpAttributeEntryWidget> EntryClass)
{
	return Cast<UMVLevelUpAttributeEntryWidget>(CreateEntry(EntryClass.Get()));
}

UMVLevelUpStatComparisonWidget* UMVLevelUpStatBlockWidget::AddComparisonEntry(
	TSubclassOf<UMVLevelUpStatComparisonWidget> EntryClass)
{
	return Cast<UMVLevelUpStatComparisonWidget>(CreateEntry(EntryClass.Get()));
}

UUserWidget* UMVLevelUpStatBlockWidget::CreateEntry(TSubclassOf<UUserWidget> EntryClass)
{
	if (IsDesignTime() || !StatBox || !EntryClass)
	{
		return nullptr;
	}

	APlayerController* OwningPlayer = GetOwningPlayer();

	if (!OwningPlayer)
	{
		return nullptr;
	}

	UUserWidget* Entry = CreateWidget<UUserWidget>(OwningPlayer, EntryClass);

	if (!Entry)
	{
		return nullptr;
	}

	UVerticalBoxSlot* EntrySlot = StatBox->AddChildToVerticalBox(Entry);

	if (!EntrySlot)
	{
		return nullptr;
	}

	EntrySlot->SetPadding(EntryPadding);
	EntrySlot->SetHorizontalAlignment(HAlign_Fill);

	return Entry;
}