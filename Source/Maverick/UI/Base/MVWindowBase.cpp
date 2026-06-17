#include "UI/Base/MVWindowBase.h"

UMVWindowBase::UMVWindowBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DesiredInputMode = ECommonInputMode::Menu;
	DesiredMouseCaptureMode = EMouseCaptureMode::NoCapture;
	DesiredMouseLockMode = EMouseLockMode::DoNotLock;
	bHideCursorDuringViewportCapture = false;
	bIgnoreMoveInput = true;
	bIgnoreLookInput = true;
}
