#include "UI/Notification/MVItemAcquisitionPanel.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
constexpr int32 MVAcquisitionVisibleLimit = 3;
constexpr float MVAcquisitionBottomMargin = 154.0f;
constexpr float MVAcquisitionCardOffset = 14.0f;
constexpr float MVAcquisitionCardMoveSeconds = 0.12f;
constexpr float MVAcquisitionCardHoldSeconds = 2.0f;
constexpr float MVAcquisitionFeedHoldSeconds = 3.0f;
}

TOptional<FMVItemAcquisitionDisplayData> FMVItemAcquisitionQueue::GetUnit(int32 Index) const
{
	for (const FMVItemAcquisitionDisplayData& Batch : Batches)
	{
		if (Index < Batch.Quantity)
		{
			FMVItemAcquisitionDisplayData Unit = Batch;
			Unit.Quantity = 1;
			Unit.bFirstAcquisition = Batch.bFirstAcquisition && Index == 0;
			return Unit;
		}
		Index -= Batch.Quantity;
	}
	return {};
}

void FMVItemAcquisitionQueue::PopFront()
{
	FMVItemAcquisitionDisplayData& Batch = Batches.First();
	if (--Batch.Quantity == 0)
	{
		Batches.PopFront();
	}
	else
	{
		Batch.bFirstAcquisition = false;
	}
}

void UMVItemAcquisitionPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("AcquisitionLayout"));
	WidgetTree->RootWidget = Root;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	CardLayer = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("AcquisitionCards"));
	UCanvasPanelSlot* CardLayerSlot = Root->AddChildToCanvas(CardLayer);
	CardLayerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	CardLayerSlot->SetOffsets(FMargin(0.0f));

	Feed = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AcquisitionFeed"));
	UCanvasPanelSlot* FeedSlot = Root->AddChildToCanvas(Feed);
	FeedSlot->SetAnchors(FAnchors(1.0f, 1.0f));
	FeedSlot->SetAlignment(FVector2D(1.0f, 1.0f));
	FeedSlot->SetPosition(FVector2D(0.0f, -112.0f));
	FeedSlot->SetAutoSize(true);
}

void UMVItemAcquisitionPanel::Present(const TSharedRef<FMVItemAcquisitionQueues>& InQueues)
{
	Queues = InQueues;
	RefreshVisibleEntries();
}

void UMVItemAcquisitionPanel::RefreshVisibleEntries()
{
	if (!Queues || !CardLayer || !Feed)
	{
		return;
	}

	while (CardLayer->GetChildrenCount() < MVAcquisitionVisibleLimit)
	{
		const int32 Index = CardLayer->GetChildrenCount();
		const auto Unit = Queues->Cards.GetUnit(Index);
		auto* Card = Unit ? AddCard(*Unit) : nullptr;
		if (!Card)
		{
			break;
		}
		if (Index == 0)
		{
			CardLifetime = { MVAcquisitionCardHoldSeconds + Card->PlayArrival() };
		}
	}
	while (Feed->GetChildrenCount() < MVAcquisitionVisibleLimit)
	{
		const auto Unit = Queues->Feed.GetUnit(Feed->GetChildrenCount());
		auto* Row = Unit ? AddFeedEntry(*Unit) : nullptr;
		if (!Row)
		{
			break;
		}
		Row->PlayArrival();
		FeedLifetimes.Add({ MVAcquisitionFeedHoldSeconds });
	}
}

void UMVItemAcquisitionPanel::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!Queues || (Queues->Cards.IsEmpty() && Queues->Feed.IsEmpty()))
	{
		return;
	}

	AdvanceCards(InDeltaTime);
	AdvanceFeed(InDeltaTime);
	RefreshVisibleEntries();
}

bool UMVItemAcquisitionPanel::FEntryLifetime::Advance(UMVItemAcquisitionEntry& Entry, const float Delta)
{
	Remaining -= Delta;
	if (Remaining > 0.0f)
	{
		return false;
	}
	if (!bDeparting)
	{
		bDeparting = true;
		Remaining = Entry.PlayDeparture();
	}
	return Remaining <= 0.0f;
}

void UMVItemAcquisitionPanel::AdvanceCards(const float Delta)
{
	if (CardMoveRemaining > 0.0f)
	{
		const float PreviousRemaining = CardMoveRemaining;
		CardMoveRemaining = FMath::Max(0.0f, CardMoveRemaining - Delta);
		for (UWidget* Card : CardLayer->GetAllChildren())
		{
			Card->SetRenderTranslation(Card->GetRenderTransform().Translation * (CardMoveRemaining / PreviousRemaining));
		}
	}

	auto* FrontCard = Cast<UMVItemAcquisitionEntry>(CardLayer->GetChildAt(0));
	if (FrontCard && CardLifetime.Advance(*FrontCard, Delta))
	{
		CardLayer->RemoveChildAt(0);
		Queues->Cards.PopFront();
		UpdateCardSlots(true);
		CardLifetime = { MVAcquisitionCardHoldSeconds + CardMoveRemaining };
	}
}

void UMVItemAcquisitionPanel::AdvanceFeed(const float Delta)
{
	int32 FinishedRows = 0;
	for (int32 Index = 0; Index < FeedLifetimes.Num(); ++Index)
	{
		auto* Row = CastChecked<UMVItemAcquisitionEntry>(Feed->GetChildAt(Index));
		FinishedRows += FeedLifetimes[Index].Advance(*Row, Delta) ? 1 : 0;
	}
	// 각 줄의 표시 시간이 같으므로 먼저 보인 줄부터 만료된다.
	for (int32 Index = 0; Index < FinishedRows; ++Index)
	{
		Feed->RemoveChildAt(0);
		Queues->Feed.PopFront();
	}
	FeedLifetimes.RemoveAt(0, FinishedRows, EAllowShrinking::No);
}

UMVItemAcquisitionEntry* UMVItemAcquisitionPanel::AddCard(const FMVItemAcquisitionDisplayData& Data)
{
	if (!CardLayer || !CardClass || CardLayer->GetChildrenCount() >= MVAcquisitionVisibleLimit)
	{
		return nullptr;
	}

	auto* Entry = CreateWidget<UMVItemAcquisitionEntry>(this, CardClass);
	if (!Entry)
	{
		return nullptr;
	}

	Entry->SetDisplayData(Data);
	UCanvasPanelSlot* EntrySlot = CardLayer->AddChildToCanvas(Entry);
	EntrySlot->SetAnchors(FAnchors(0.5f, 1.0f));
	EntrySlot->SetAlignment(FVector2D(0.5f, 1.0f));
	EntrySlot->SetAutoSize(true);

	UpdateCardSlots(false);
	return Entry;
}

void UMVItemAcquisitionPanel::UpdateCardSlots(const bool bAnimate)
{
	if (bAnimate)
	{
		CardMoveRemaining = CardLayer->HasAnyChildren() ? MVAcquisitionCardMoveSeconds : 0.0f;
	}
	for (int32 Index = 0; Index < CardLayer->GetChildrenCount(); ++Index)
	{
		auto* Card = CastChecked<UMVItemAcquisitionEntry>(CardLayer->GetChildAt(Index));
		Card->SetCardInFront(Index == 0);
		auto* CardSlot = CastChecked<UCanvasPanelSlot>(Card->Slot);
		const float TargetY = -MVAcquisitionBottomMargin - MVAcquisitionCardOffset * Index;
		if (bAnimate)
		{
			Card->SetRenderTranslation(FVector2D(0.0f, CardSlot->GetPosition().Y - TargetY));
		}
		CardSlot->SetZOrder(MVAcquisitionVisibleLimit - Index);
		CardSlot->SetPosition(FVector2D(0.0f, TargetY));
	}
}

UMVItemAcquisitionEntry* UMVItemAcquisitionPanel::AddFeedEntry(const FMVItemAcquisitionDisplayData& Data)
{
	if (!Feed || !FeedEntryClass || Feed->GetChildrenCount() >= MVAcquisitionVisibleLimit)
	{
		return nullptr;
	}

	auto* Entry = CreateWidget<UMVItemAcquisitionEntry>(this, FeedEntryClass);
	if (Entry)
	{
		Entry->SetDisplayData(Data);
		Feed->AddChildToVerticalBox(Entry)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 3.0f));
	}
	return Entry;
}
