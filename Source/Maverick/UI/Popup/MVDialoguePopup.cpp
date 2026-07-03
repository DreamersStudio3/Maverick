#include "UI/Popup/MVDialoguePopup.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

UMVDialoguePopup::UMVDialoguePopup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AutoDismissSeconds = 0.0f;
}

void UMVDialoguePopup::SetDialogueText(FText InDialogueText)
{
	if (DialogueText)
	{
		DialogueText->SetText(InDialogueText);
		DialogueText->SetRenderOpacity(1.0f);
	}
}

void UMVDialoguePopup::SetDialogueAutoDismissSeconds(float InAutoDismissSeconds)
{
	AutoDismissSeconds = FMath::Max(0.0f, InAutoDismissSeconds);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}
	if (bConstructed && GetParent())
	{
		StartAutoDismissTimer();
	}
}

void UMVDialoguePopup::SetMinimumSkipDelay(float InMinimumSkipDelay)
{
	MinimumSkipDelay = FMath::Max(0.0f, InMinimumSkipDelay);
	bCanSkipDialogue = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MinimumSkipDelayTimerHandle);
	}

	if (bConstructed && GetParent())
	{
		StartMinimumSkipDelayTimer();
	}
}

void UMVDialoguePopup::CloseDialogue()
{
	if (bCloseRequested)
	{
		return;
	}

	bCloseRequested = true;
	bCanSkipDialogue = false;
	ClearDialogueTimers();

	if (DialogueText && DialogueTextFadeOutSeconds > 0.0f && DialogueText->GetRenderOpacity() > 0.0f)
	{
		DialogueTextFadeController.Play(
			*DialogueText,
			DialogueText->GetRenderOpacity(),
			0.0f,
			DialogueTextFadeOutSeconds,
			true,
			[this]()
			{
				HandleDialogueTextFadeOutFinished();
			});
		return;
	}

	HandleDialogueTextFadeOutFinished();
}

bool UMVDialoguePopup::CanSkipDialogue() const
{
	return bCanSkipDialogue
		&& GetParent()
		&& !bCloseRequested
		&& !IsFading()
		&& !DialogueTextFadeController.IsPlaying();
}

void UMVDialoguePopup::NativeConstruct()
{
	if (!WidgetTree || !WidgetTree->RootWidget)
	{
		BuildNativeDialogueTree();
	}

	SetUIFadeDurations(DialoguePopupFadeSeconds, DialoguePopupFadeSeconds);

	bClosedEventBroadcast = false;
	bClosingEventBroadcast = false;
	bCloseRequested = false;
	bCanSkipDialogue = false;
	bConstructed = true;
	DialogueTextFadeController.Stop();
	if (DialogueText)
	{
		DialogueText->SetRenderOpacity(1.0f);
	}
	ClearDialogueTimers();

	Super::NativeConstruct();

	StartMinimumSkipDelayTimer();
	StartAutoDismissTimer();
}

void UMVDialoguePopup::NativeDestruct()
{
	ClearDialogueTimers();
	DialogueTextFadeController.Stop();

	bConstructed = false;
	bCanSkipDialogue = false;
	BroadcastDialogueClosing();

	Super::NativeDestruct();

	BroadcastDialogueClosed();
}

void UMVDialoguePopup::BuildNativeDialogueTree()
{
	if (!WidgetTree)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(),
		TEXT("DialoguePopupRoot"));
	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(),
		TEXT("DialoguePopupPanel"));
	DialogueText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(),
		TEXT("DialoguePopupText"));

	if (!RootCanvas || !PanelBorder || !DialogueText)
	{
		return;
	}

	WidgetTree->RootWidget = RootCanvas;
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PanelBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	PanelBorder->SetPadding(FMargin(24.0f, 18.0f));
	PanelBorder->SetBrushColor(FLinearColor(0.015f, 0.014f, 0.012f, 0.82f));
	PanelBorder->SetContent(DialogueText);

	DialogueText->SetJustification(ETextJustify::Center);
	DialogueText->SetAutoWrapText(true);
	DialogueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.84f, 0.75f, 1.0f)));

	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBorder))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		PanelSlot->SetPosition(FVector2D(0.0f, -74.0f));
		PanelSlot->SetSize(FVector2D(860.0f, 132.0f));
	}
}

void UMVDialoguePopup::HandleAutoDismissElapsed()
{
	CloseDialogue();
}

void UMVDialoguePopup::HandleMinimumSkipDelayElapsed()
{
	bCanSkipDialogue = true;
}

void UMVDialoguePopup::HandleDialogueTextFadeOutFinished()
{
	BeginDialoguePopupFadeOut();
}

void UMVDialoguePopup::StartAutoDismissTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);

		if (AutoDismissSeconds > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				AutoDismissTimerHandle,
				this,
				&UMVDialoguePopup::HandleAutoDismissElapsed,
				AutoDismissSeconds,
				false);
		}
	}
}

void UMVDialoguePopup::StartMinimumSkipDelayTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MinimumSkipDelayTimerHandle);

		if (MinimumSkipDelay > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				MinimumSkipDelayTimerHandle,
				this,
				&UMVDialoguePopup::HandleMinimumSkipDelayElapsed,
				MinimumSkipDelay,
				false);
			return;
		}
	}

	bCanSkipDialogue = true;
}

void UMVDialoguePopup::ClearDialogueTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
		World->GetTimerManager().ClearTimer(MinimumSkipDelayTimerHandle);
	}
}

void UMVDialoguePopup::BeginDialoguePopupFadeOut()
{
	BroadcastDialogueClosing();
	ClosePopup();
}

void UMVDialoguePopup::BroadcastDialogueClosing()
{
	if (bClosingEventBroadcast)
	{
		return;
	}

	bClosingEventBroadcast = true;
	OnDialoguePopupClosing.Broadcast(this);
}

void UMVDialoguePopup::BroadcastDialogueClosed()
{
	if (bClosedEventBroadcast)
	{
		return;
	}

	bClosedEventBroadcast = true;
	OnDialoguePopupClosed.Broadcast(this);
}
