#include "UI/Window/MVInteractionMenuWindow.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "UI/System/MVUISubsystem.h"

UMVInteractionMenuEntryButton::UMVInteractionMenuEntryButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	OnClicked.AddDynamic(this, &UMVInteractionMenuEntryButton::HandleClicked);
}

void UMVInteractionMenuEntryButton::SetEntryData(const FMVMenuEntryData& InEntryData)
{
	EntryData = InEntryData;
	SetIsEnabled(EntryData.bEnabled);
}

void UMVInteractionMenuEntryButton::HandleClicked()
{
	OnEntryButtonClicked.Broadcast(this);
}

UMVInteractionMenuWindow::UMVInteractionMenuWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCloseOnBack = true;
}

void UMVInteractionMenuWindow::SetMenuData(const FMVInteractionMenuData& InMenuData, UObject* InSourceObject)
{
	MenuData = InMenuData;
	MenuData.NormalizeEntryParentMenuIds();
	SourceObject = InSourceObject;
	MenuStack.Reset();
	CurrentMenuId = MenuData.RootMenuId;
	RefreshMenu();
}

void UMVInteractionMenuWindow::RefreshMenu()
{
	if (!EntryBox)
	{
		return;
	}

	if (TitleText)
	{
		TitleText->SetText(ResolveCurrentTitle());
	}

	EntryBox->ClearChildren();

	if (!MenuStack.IsEmpty())
	{
		FMVMenuEntryData BackEntry;
		BackEntry.Label = NSLOCTEXT("MaverickInteractionMenu", "Back", "Back");
		BackEntry.bCloseMenuOnExecute = false;
		BackEntry.bInternalBackEntry = true;

		UMVInteractionMenuEntryButton* BackButton =
			WidgetTree->ConstructWidget<UMVInteractionMenuEntryButton>(
				UMVInteractionMenuEntryButton::StaticClass(),
				TEXT("InteractionMenuBackButton"));
		if (BackButton)
		{
			BackButton->SetEntryData(BackEntry);
			BackButton->OnEntryButtonClicked.AddUniqueDynamic(this, &UMVInteractionMenuWindow::HandleEntryButtonClicked);

			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InteractionMenuBackText"));
			if (ButtonText)
			{
				ButtonText->SetText(BackEntry.Label);
				BackButton->SetContent(ButtonText);
			}

			if (UVerticalBoxSlot* EntrySlot = EntryBox->AddChildToVerticalBox(BackButton))
			{
				EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
			}
		}
	}

	const TArray<FMVMenuEntryData> Entries = GetCurrentEntries();
	if (Entries.IsEmpty())
	{
		UTextBlock* EmptyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InteractionMenuEmptyText"));
		if (EmptyText)
		{
			EmptyText->SetText(NSLOCTEXT("MaverickInteractionMenu", "Empty", "No entries"));
			EntryBox->AddChildToVerticalBox(EmptyText);
		}
		return;
	}

	int32 EntryIndex = 0;
	for (const FMVMenuEntryData& EntryData : Entries)
	{
		const FName ButtonName = *FString::Printf(TEXT("InteractionMenuEntry_%d"), EntryIndex++);
		UMVInteractionMenuEntryButton* EntryButton =
			WidgetTree->ConstructWidget<UMVInteractionMenuEntryButton>(
				UMVInteractionMenuEntryButton::StaticClass(),
				ButtonName);
		if (!EntryButton)
		{
			continue;
		}

		EntryButton->SetEntryData(EntryData);
		EntryButton->OnEntryButtonClicked.AddUniqueDynamic(this, &UMVInteractionMenuWindow::HandleEntryButtonClicked);

		UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%s_Text"), *ButtonName.ToString()));
		if (ButtonText)
		{
			ButtonText->SetText(ResolveEntryLabel(EntryData));
			ButtonText->SetJustification(ETextJustify::Left);
			EntryButton->SetContent(ButtonText);
		}

		if (UVerticalBoxSlot* EntrySlot = EntryBox->AddChildToVerticalBox(EntryButton))
		{
			EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
	}
}

void UMVInteractionMenuWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		BuildNativeMenuTree();
	}

	RefreshMenu();
}

void UMVInteractionMenuWindow::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (bClosedEventBroadcast)
	{
		return;
	}

	bClosedEventBroadcast = true;
	OnInteractionMenuClosed.Broadcast(this);
}

bool UMVInteractionMenuWindow::NativeOnHandleBackAction()
{
	if (NavigateBack())
	{
		return true;
	}

	return Super::NativeOnHandleBackAction();
}

void UMVInteractionMenuWindow::BuildNativeMenuTree()
{
	if (!WidgetTree)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InteractionMenuRoot"));
	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InteractionMenuPanel"));
	UVerticalBox* PanelBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InteractionMenuPanelBox"));
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InteractionMenuTitle"));
	EntryBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InteractionMenuEntryBox"));

	if (!RootCanvas || !PanelBorder || !PanelBox || !TitleText || !EntryBox)
	{
		return;
	}

	WidgetTree->RootWidget = RootCanvas;

	PanelBorder->SetPadding(FMargin(18.0f, 16.0f));
	PanelBorder->SetBrushColor(FLinearColor(0.025f, 0.028f, 0.03f, 0.88f));
	PanelBorder->SetContent(PanelBox);

	TitleText->SetText(ResolveCurrentTitle());

	if (UVerticalBoxSlot* TitleSlot = PanelBox->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));
	}
	PanelBox->AddChildToVerticalBox(EntryBox);

	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBorder))
	{
		PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
		PanelSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		PanelSlot->SetOffsets(FMargin(48.0f, 96.0f, 360.0f, 96.0f));
	}
}

bool UMVInteractionMenuWindow::NavigateBack()
{
	if (MenuStack.IsEmpty())
	{
		return false;
	}

	CurrentMenuId = MenuStack.Pop();
	RefreshMenu();
	return true;
}

TArray<FMVMenuEntryData> UMVInteractionMenuWindow::GetCurrentEntries() const
{
	TArray<FMVMenuEntryData> Result;
	for (const FMVMenuEntryData& EntryData : MenuData.Entries)
	{
		if (!CurrentMenuId.IsValid())
		{
			if (!EntryData.ParentMenuId.IsValid())
			{
				Result.Add(EntryData);
			}
			continue;
		}

		if (EntryData.ParentMenuId == CurrentMenuId
			|| (CurrentMenuId == MenuData.RootMenuId && !EntryData.ParentMenuId.IsValid()))
		{
			Result.Add(EntryData);
		}
	}

	for (const FMVInteractionMenuPageData& SubMenu : MenuData.SubMenus)
	{
		if (SubMenu.MenuId == CurrentMenuId)
		{
			Result.Append(SubMenu.Entries);
			break;
		}
	}

	return Result;
}

FText UMVInteractionMenuWindow::ResolveCurrentTitle() const
{
	for (const FMVInteractionMenuPageData& SubMenu : MenuData.SubMenus)
	{
		if (SubMenu.MenuId == CurrentMenuId && !SubMenu.Title.IsEmpty())
		{
			return SubMenu.Title;
		}
	}

	return MenuData.Title;
}

FText UMVInteractionMenuWindow::ResolveEntryLabel(const FMVMenuEntryData& EntryData) const
{
	if (EntryData.bEnabled)
	{
		return EntryData.Label;
	}

	return FText::Format(
		NSLOCTEXT("MaverickInteractionMenu", "DisabledEntryFormat", "{0}"),
		EntryData.Label);
}

void UMVInteractionMenuWindow::HandleEntryButtonClicked(UMVInteractionMenuEntryButton* Button)
{
	if (!Button)
	{
		return;
	}

	const FMVMenuEntryData& EntryData = Button->GetEntryData();
	if (!EntryData.bEnabled)
	{
		return;
	}

	if (EntryData.bInternalBackEntry)
	{
		NavigateBack();
		return;
	}

	if (EntryData.SubMenuId.IsValid())
	{
		MenuStack.Add(CurrentMenuId);
		CurrentMenuId = EntryData.SubMenuId;
		RefreshMenu();
		return;
	}

	if (EntryData.WindowClass)
	{
		UWorld* World = GetWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
		if (UISubsystem)
		{
			UISubsystem->PushWindowByClass(EntryData.WindowClass);
		}
	}

	const FName SelectionName = EntryData.ResolveSelectionName();
	if (!SelectionName.IsNone())
	{
		OnInteractionMenuEntrySelected.Broadcast(SourceObject, EntryData);
	}

	if (EntryData.bCloseMenuOnExecute)
	{
		DeactivateWidgetWithFade();
	}
}
