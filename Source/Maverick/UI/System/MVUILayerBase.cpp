#include "UI/System/MVUILayerBase.h"

#include "Blueprint/WidgetTree.h"
#include "CommonActivatableWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UMVUILayerBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		BuildNativeLayerTree();
	}
}

UCommonActivatableWidget* UMVUILayerBase::PushWindow(TSubclassOf<UCommonActivatableWidget> WindowClass)
{
	if (!WindowStack || !WindowClass)
	{
		return nullptr;
	}

	return WindowStack->AddWidget(WindowClass);
}

UCommonActivatableWidget* UMVUILayerBase::PushPopup(TSubclassOf<UCommonActivatableWidget> PopupClass)
{
	if (!PopupStack || !PopupClass)
	{
		return nullptr;
	}

	return PopupStack->AddWidget(PopupClass);
}

UUserWidget* UMVUILayerBase::SetHUDByClass(TSubclassOf<UUserWidget> HUDClass, APlayerController* OwningPlayer)
{
	if (!HUDClass)
	{
		return nullptr;
	}

	UUserWidget* NewHUD = OwningPlayer
		? CreateWidget<UUserWidget>(OwningPlayer, HUDClass)
		: CreateWidget<UUserWidget>(GetWorld(), HUDClass);
	SetHUDWidget(NewHUD);
	return NewHUD;
}

void UMVUILayerBase::SetHUDWidget(UUserWidget* InHUDWidget)
{
	if (!HUDLayer)
	{
		return;
	}

	HUDLayer->ClearChildren();
	ActiveHUDWidget = InHUDWidget;

	if (!ActiveHUDWidget)
	{
		return;
	}

	if (UOverlaySlot* HUDSlot = HUDLayer->AddChildToOverlay(ActiveHUDWidget))
	{
		HUDSlot->SetHorizontalAlignment(HAlign_Fill);
		HUDSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UMVUILayerBase::AddWidgetLayerWidget(UUserWidget* InWidget)
{
	if (!WidgetLayer || !InWidget)
	{
		return;
	}

	if (UOverlaySlot* WidgetSlot = WidgetLayer->AddChildToOverlay(InWidget))
	{
		WidgetSlot->SetHorizontalAlignment(HAlign_Fill);
		WidgetSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UMVUILayerBase::ClearLayer()
{
	if (WindowStack)
	{
		WindowStack->ClearWidgets();
	}

	if (HUDLayer)
	{
		HUDLayer->ClearChildren();
	}

	if (PopupStack)
	{
		PopupStack->ClearWidgets();
	}

	if (WidgetLayer)
	{
		WidgetLayer->ClearChildren();
	}

	ActiveHUDWidget = nullptr;
}

void UMVUILayerBase::BuildNativeLayerTree()
{
	if (!WidgetTree)
	{
		return;
	}

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	WindowStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(
		UCommonActivatableWidgetStack::StaticClass(),
		TEXT("WindowStack"));
	HUDLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("HUDLayer"));
	PopupStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(
		UCommonActivatableWidgetStack::StaticClass(),
		TEXT("PopupStack"));
	WidgetLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("WidgetLayer"));

	WidgetTree->RootWidget = RootOverlay;

	AddFullScreenOverlayChild(RootOverlay, WindowStack);
	AddFullScreenOverlayChild(RootOverlay, HUDLayer);
	AddFullScreenOverlayChild(RootOverlay, PopupStack);
	AddFullScreenOverlayChild(RootOverlay, WidgetLayer);
}

void UMVUILayerBase::AddFullScreenOverlayChild(UOverlay* RootOverlay, UWidget* ChildWidget) const
{
	if (!RootOverlay || !ChildWidget)
	{
		return;
	}

	if (UOverlaySlot* ChildSlot = RootOverlay->AddChildToOverlay(ChildWidget))
	{
		ChildSlot->SetHorizontalAlignment(HAlign_Fill);
		ChildSlot->SetVerticalAlignment(VAlign_Fill);
	}
}
