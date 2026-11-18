#include "UI/System/MVGameplayInputWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Engine/LocalPlayer.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Input/CommonUIInputTypes.h"

UMVGameplayInputWidget::UMVGameplayInputWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;
	bIsBackHandler = false;
	bSupportsActivationFocus = true;
	bIsModal = true;
	bAutoRestoreFocus = false;

	SetIsFocusable(false);

	bSetVisibilityOnActivated = true;
	ActivatedVisibility = ESlateVisibility::SelfHitTestInvisible;

	bSetVisibilityOnDeactivated = true;
	DeactivatedVisibility = ESlateVisibility::Collapsed;
}

void UMVGameplayInputWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(),
			TEXT("GameplayInputRoot"));
	}
}

TOptional<FUIInputConfig> UMVGameplayInputWidget::GetDesiredInputConfig() const
{
	if (bMenuInputBlocked)
	{
		FUIInputConfig MenuConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture,
			EMouseLockMode::DoNotLock, false);
		MenuConfig.bIgnoreMoveInput = true;
		MenuConfig.bIgnoreLookInput = true;
		return MenuConfig;
	}

	FUIInputConfig InputConfig(
		ECommonInputMode::Game,
		EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown,
		EMouseLockMode::LockOnCapture,
		true);

	InputConfig.bIgnoreMoveInput = false;
	InputConfig.bIgnoreLookInput = false;

	return InputConfig;
}

void UMVGameplayInputWidget::SetMenuInputBlocked(const bool bBlocked)
{
	if (bMenuInputBlocked == bBlocked)
	{
		return;
	}
	bMenuInputBlocked = bBlocked;
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UCommonUIActionRouterBase* Router = LocalPlayer ? LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>() : nullptr;
	// 첫 창이 활성화되기 전에도 차단한다. 해제는 이 기본 위젯이 입력을 맡은 경우에만 적용한다.
	if (Router && (bBlocked || Router->GetLeafmostActivatableWidget() == this))
	{
		Router->SetActiveUIInputConfig(GetDesiredInputConfig().GetValue(), this);
	}
}
