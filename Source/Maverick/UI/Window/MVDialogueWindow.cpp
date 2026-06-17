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
	}
}

void UMVDialogueWindow::SetAutoDismissSeconds(float InAutoDismissSeconds)
{
	AutoDismissSeconds = FMath::Max(0.0f, InAutoDismissSeconds);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}
	StartAutoDismissTimer();
}

void UMVDialogueWindow::CloseDialogue()
{
	DeactivateWidgetWithFade();
}

void UMVDialogueWindow::NativeOnActivated()
{
	Super::NativeOnActivated();

	bClosedEventBroadcast = false;
	StartAutoDismissTimer();
}

void UMVDialogueWindow::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}

	Super::NativeOnDeactivated();

	RestoreGameInputMode();
	BroadcastDialogueClosed();
}

void UMVDialogueWindow::HandleAutoDismissElapsed()
{
	CloseDialogue();
}

void UMVDialogueWindow::StartAutoDismissTimer()
{
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
