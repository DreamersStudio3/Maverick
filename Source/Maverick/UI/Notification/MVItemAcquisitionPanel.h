#pragma once

#include "Containers/RingBuffer.h"
#include "UI/Notification/MVItemAcquisitionEntry.h"
#include "MVItemAcquisitionPanel.generated.h"

class UCanvasPanel;
class UVerticalBox;

/** 지급 단위로 데이터를 보관하고 화면에 올릴 때 아이템 한 개씩 꺼낸다. */
class FMVItemAcquisitionQueue
{
public:
	void Add(const FMVItemAcquisitionDisplayData& Batch) { Batches.Add(Batch); }
	bool IsEmpty() const { return Batches.IsEmpty(); }
	TOptional<FMVItemAcquisitionDisplayData> GetUnit(int32 Index) const;
	void PopFront();

private:
	TRingBuffer<FMVItemAcquisitionDisplayData> Batches;
};

/** 각 위치에서 표시를 마칠 때까지 습득 데이터를 보유한다. 화면 교체 시에도 유지된다. */
struct FMVItemAcquisitionQueues
{
	FMVItemAcquisitionQueue Cards;
	FMVItemAcquisitionQueue Feed;
};

/** 두 대기열을 순서대로 표시하고 페이드가 끝난 항목을 비운다. */
UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVItemAcquisitionPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	void Present(const TSharedRef<FMVItemAcquisitionQueues>& InQueues);
	void RefreshVisibleEntries();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|UI|Acquisition")
	TSubclassOf<UMVItemAcquisitionEntry> CardClass;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|UI|Acquisition")
	TSubclassOf<UMVItemAcquisitionEntry> FeedEntryClass;

private:
	struct FEntryLifetime
	{
		float Remaining = 0.0f;
		bool bDeparting = false;

		bool Advance(UMVItemAcquisitionEntry& Entry, float Delta);
	};

	UMVItemAcquisitionEntry* AddCard(const FMVItemAcquisitionDisplayData& Data);
	UMVItemAcquisitionEntry* AddFeedEntry(const FMVItemAcquisitionDisplayData& Data);
	void UpdateCardSlots(bool bAnimate);
	void AdvanceCards(float Delta);
	void AdvanceFeed(float Delta);

	TSharedPtr<FMVItemAcquisitionQueues> Queues;
	FEntryLifetime CardLifetime;
	TArray<FEntryLifetime> FeedLifetimes;
	float CardMoveRemaining = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> CardLayer;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> Feed;
};
