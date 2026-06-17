#include "UI/Window/MVDeathOverlayWindow.h"

#include "UI/System/MVUISubsystem.h"

UMVDeathOverlayWindow::UMVDeathOverlayWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCloseOnBack = false;
	DesiredInputMode = ECommonInputMode::All;
	bIgnoreMoveInput = true;
	bIgnoreLookInput = true;
}

void UMVDeathOverlayWindow::SetDisplaySeconds(float InDisplaySeconds)
{
	DisplaySeconds = FMath::Max(0.0f, InDisplaySeconds);
}

void UMVDeathOverlayWindow::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DisplayTimerHandle,
			this,
			&UMVDeathOverlayWindow::HandleDisplayTimeElapsed,
			DisplaySeconds,
			false);
	}
}

void UMVDeathOverlayWindow::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DisplayTimerHandle);
	}

	Super::NativeOnDeactivated();
}

void UMVDeathOverlayWindow::HandleDisplayTimeElapsed()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMVUISubsystem* UISubsystem = GameInstance->GetSubsystem<UMVUISubsystem>())
		{
			UISubsystem->ShowLoadingWindow();
		}
	}
}
