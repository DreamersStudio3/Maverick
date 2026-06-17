#include "UI/Window/MVDialogueWindow.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

UMVDialogueWindow::UMVDialogueWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DesiredInputMode = ECommonInputMode::All;
	DesiredMouseCaptureMode = EMouseCaptureMode::NoCapture;
	DesiredMouseLockMode = EMouseLockMode::DoNotLock;
	bHideCursorDuringViewportCapture = false;
	bIgnoreMoveInput = false;
	bIgnoreLookInput = false;
}

void UMVDialogueWindow::SetDialogueText(FText InDialogueText)
{
	if (DialogueText)
	{
		DialogueText->SetText(InDialogueText);
		DialogueText->SetRenderOpacity(1.0f);
	}
}

void UMVDialogueWindow::SetAutoDismissSeconds(float InAutoDismissSeconds)
{
	AutoDismissSeconds = FMath::Max(0.0f, InAutoDismissSeconds);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}
	if (bFadeInFinished && IsActivated())
	{
		StartAutoDismissTimer();
	}
}

void UMVDialogueWindow::SetMinimumSkipDelay(float InMinimumSkipDelay)
{
	MinimumSkipDelay = FMath::Max(0.0f, InMinimumSkipDelay);
	bCanSkipDialogue = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MinimumSkipDelayTimerHandle);
	}

	if (bFadeInFinished && IsActivated())
	{
		StartMinimumSkipDelayTimer();
	}
}

void UMVDialogueWindow::CloseDialogue()
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

bool UMVDialogueWindow::CanSkipDialogue() const
{
	return bCanSkipDialogue
		&& IsActivated()
		&& !bCloseRequested
		&& !IsFading()
		&& !DialogueTextFadeController.IsPlaying();
}

void UMVDialogueWindow::NativeOnActivated()
{
	SetUIFadeDurations(DialogueWindowFadeSeconds, DialogueWindowFadeSeconds);

	bClosedEventBroadcast = false;
	bClosingEventBroadcast = false;
	bCloseRequested = false;
	bFadeInFinished = false;
	bCanSkipDialogue = false;
	DialogueTextFadeController.Stop();
	if (DialogueText)
	{
		DialogueText->SetRenderOpacity(1.0f);
	}
	ClearDialogueTimers();

	Super::NativeOnActivated();
}

void UMVDialogueWindow::NativeOnDeactivated()
{
	ClearDialogueTimers();
	DialogueTextFadeController.Stop();

	bFadeInFinished = false;
	bCanSkipDialogue = false;
	BroadcastDialogueClosing();

	Super::NativeOnDeactivated();

	RestoreGameInputMode();
	BroadcastDialogueClosed();
}

void UMVDialogueWindow::HandleFadeInFinished()
{
	Super::HandleFadeInFinished();

	bFadeInFinished = true;
	StartMinimumSkipDelayTimer();
	StartAutoDismissTimer();
}

void UMVDialogueWindow::HandleAutoDismissElapsed()
{
	CloseDialogue();
}

bool UMVDialogueWindow::NativeOnHandleBackAction()
{
	if (!bCloseOnBack)
	{
		return false;
	}

	CloseDialogue();
	return true;
}

void UMVDialogueWindow::HandleMinimumSkipDelayElapsed()
{
	bCanSkipDialogue = true;
}

void UMVDialogueWindow::HandleDialogueTextFadeOutFinished()
{
	BeginDialogueWindowFadeOut();
}

void UMVDialogueWindow::StartAutoDismissTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}

	if (AutoDismissSeconds > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AutoDismissTimerHandle,
				this,
				&UMVDialogueWindow::HandleAutoDismissElapsed,
				AutoDismissSeconds,
				false);
		}
	}
}

void UMVDialogueWindow::StartMinimumSkipDelayTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MinimumSkipDelayTimerHandle);

		if (MinimumSkipDelay > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				MinimumSkipDelayTimerHandle,
				this,
				&UMVDialogueWindow::HandleMinimumSkipDelayElapsed,
				MinimumSkipDelay,
				false);
			return;
		}
	}

	bCanSkipDialogue = true;
}

void UMVDialogueWindow::ClearDialogueTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
		World->GetTimerManager().ClearTimer(MinimumSkipDelayTimerHandle);
	}
}

void UMVDialogueWindow::BeginDialogueWindowFadeOut()
{
	BroadcastDialogueClosing();
	DeactivateWidgetWithFade();
}

void UMVDialogueWindow::BroadcastDialogueClosing()
{
	if (bClosingEventBroadcast)
	{
		return;
	}

	bClosingEventBroadcast = true;
	OnDialogueWindowClosing.Broadcast(this);
}

void UMVDialogueWindow::BroadcastDialogueClosed()
{
	if (bClosedEventBroadcast)
	{
		return;
	}

	bClosedEventBroadcast = true;
	OnDialogueWindowClosed.Broadcast(this);
}

void UMVDialogueWindow::RestoreGameInputMode() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
}
