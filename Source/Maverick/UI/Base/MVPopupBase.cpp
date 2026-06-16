#include "UI/Base/MVPopupBase.h"

UMVPopupBase::UMVPopupBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DesiredInputMode = ECommonInputMode::All;
	bIsModal = false;
	bCloseOnBack = false;
	bIgnoreMoveInput = false;
	bIgnoreLookInput = false;
}

void UMVPopupBase::ClosePopup()
{
	DeactivateWidget();
}

void UMVPopupBase::NativeOnActivated()
{
	Super::NativeOnActivated();

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

void UMVPopupBase::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}

	Super::NativeOnDeactivated();
}

void UMVPopupBase::HandleAutoDismissElapsed()
{
	ClosePopup();
}
