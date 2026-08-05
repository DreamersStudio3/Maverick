#include "UI/Window/MVDeathOverlayWindow.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"

namespace
{
const FText MVDeathOverlayDefaultDeathText = NSLOCTEXT("MaverickDeath", "YouDied", "YOU DIED");
}

UMVDeathOverlayWindow::UMVDeathOverlayWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCloseOnBack = false;
	DesiredInputMode = ECommonInputMode::Game;
	DesiredMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
	DesiredMouseLockMode = EMouseLockMode::LockOnCapture;
	bHideCursorDuringViewportCapture = true;
	bIgnoreMoveInput = true;
	bIgnoreLookInput = false;
	ApplyDeathOverlayFadeDurations();
}

void UMVDeathOverlayWindow::SetDisplaySeconds(float InDisplaySeconds)
{
	MinimumDisplaySeconds = FMath::Max(0.0f, InDisplaySeconds);
	if (IsActivated() && bFadeInFinished)
	{
		StartMinimumDisplayTimer();
	}
}

void UMVDeathOverlayWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildNativeWidgetTree();
	RefreshDeathText();
}

void UMVDeathOverlayWindow::NativeOnActivated()
{
	DesiredInputMode = ECommonInputMode::Game;
	DesiredMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
	DesiredMouseLockMode = EMouseLockMode::LockOnCapture;
	bHideCursorDuringViewportCapture = true;
	bIgnoreMoveInput = true;
	bIgnoreLookInput = false;

	bMinimumDisplayElapsed = false;
	bFadeInFinished = false;
	bFadeOutRequested = false;
	ApplyDeathOverlayFadeDurations();
	RefreshDeathText();
	ClearMinimumDisplayTimer();

	Super::NativeOnActivated();
}

void UMVDeathOverlayWindow::NativeOnDeactivated()
{
	ClearMinimumDisplayTimer();
	bMinimumDisplayElapsed = false;
	bFadeInFinished = false;
	bFadeOutRequested = false;

	Super::NativeOnDeactivated();
}

void UMVDeathOverlayWindow::HandleFadeInFinished()
{
	Super::HandleFadeInFinished();

	bFadeInFinished = true;
	StartMinimumDisplayTimer();
}

void UMVDeathOverlayWindow::HandleFadeOutFinished()
{
	Super::HandleFadeOutFinished();

	HandleMinimumDisplayTimeElapsed();
}

void UMVDeathOverlayWindow::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DeathOverlayRoot"));
	DeathText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathText"));
	WidgetTree->RootWidget = RootOverlay;

	if (!RootOverlay || !DeathText)
	{
		return;
	}

	FSlateFontInfo DeathFont = DeathText->GetFont();
	DeathFont.Size = 72;
	DeathText->SetFont(DeathFont);
	DeathText->SetJustification(ETextJustify::Center);
	DeathText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.05f, 0.04f, 1.0f)));
	DeathText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
	DeathText->SetShadowOffset(FVector2D(3.0f, 3.0f));

	if (UOverlaySlot* DeathTextSlot = RootOverlay->AddChildToOverlay(DeathText))
	{
		DeathTextSlot->SetHorizontalAlignment(HAlign_Center);
		DeathTextSlot->SetVerticalAlignment(VAlign_Center);
		DeathTextSlot->SetPadding(FMargin(32.0f));
	}
}

void UMVDeathOverlayWindow::ApplyDeathOverlayFadeDurations()
{
	const float DeathOverlayFadeSeconds = FMath::Max(0.0f, DeathOverlayFadeInSeconds);
	DeathOverlayFadeOutSeconds = DeathOverlayFadeSeconds;
	SetUIFadeDurations(DeathOverlayFadeSeconds, DeathOverlayFadeSeconds);
}

void UMVDeathOverlayWindow::RefreshDeathText()
{
	if (DeathText)
	{
		DeathText->SetText(MVDeathOverlayDefaultDeathText);
	}
}

void UMVDeathOverlayWindow::StartMinimumDisplayTimer()
{
	ClearMinimumDisplayTimer();
	if (bFadeOutRequested || !IsActivated())
	{
		return;
	}

	if (MinimumDisplaySeconds <= 0.0f)
	{
		BeginFadeOutAfterDisplay();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MinimumDisplayTimerHandle,
			this,
			&UMVDeathOverlayWindow::BeginFadeOutAfterDisplay,
			MinimumDisplaySeconds,
			false);
	}
}

void UMVDeathOverlayWindow::ClearMinimumDisplayTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MinimumDisplayTimerHandle);
	}
}

void UMVDeathOverlayWindow::BeginFadeOutAfterDisplay()
{
	ClearMinimumDisplayTimer();
	if (bFadeOutRequested || !IsActivated())
	{
		return;
	}

	bFadeOutRequested = true;
	DeactivateWidgetWithFade();
}

void UMVDeathOverlayWindow::HandleMinimumDisplayTimeElapsed()
{
	if (bMinimumDisplayElapsed)
	{
		return;
	}

	bMinimumDisplayElapsed = true;
	OnMinimumDisplayElapsed.Broadcast();
}
