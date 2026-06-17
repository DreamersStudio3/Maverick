#include "UI/Base/MVPopupBase.h"

UMVPopupBase::UMVPopupBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVPopupBase::ClosePopup()
{
	if (IsClosing())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}

	RemoveFromParentWithFade();
}

void UMVPopupBase::ClosePopupImmediately()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}

	RemoveFromParent();
	if (!GetParent())
	{
		BroadcastPopupClosed();
	}
}

void UMVPopupBase::SetAutoDismissSeconds(float InAutoDismissSeconds)
{
	AutoDismissSeconds = FMath::Max(0.0f, InAutoDismissSeconds);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}
	StartAutoDismissTimer();
}

void UMVPopupBase::NativeConstruct()
{
	Super::NativeConstruct();

	bClosedEventBroadcast = false;
	StartAutoDismissTimer();
}

void UMVPopupBase::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}

	BroadcastPopupClosed();
	Super::NativeDestruct();
}

void UMVPopupBase::HandleFadeOutFinished()
{
	RemoveFromParent();
	if (!GetParent())
	{
		BroadcastPopupClosed();
	}
}

void UMVPopupBase::HandleAutoDismissElapsed()
{
	ClosePopup();
}

void UMVPopupBase::StartAutoDismissTimer()
{
	if (AutoDismissSeconds > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AutoDismissTimerHandle,
				this,
				&UMVPopupBase::HandleAutoDismissElapsed,
				AutoDismissSeconds,
				false);
		}
	}
}

void UMVPopupBase::BroadcastPopupClosed()
{
	if (bClosedEventBroadcast)
	{
		return;
	}

	bClosedEventBroadcast = true;
	OnPopupClosed.Broadcast(this);
}
