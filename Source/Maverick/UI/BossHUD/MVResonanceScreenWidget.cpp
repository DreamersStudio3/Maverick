#include "UI/BossHUD/MVResonanceScreenWidget.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

TSharedRef<SWidget> UMVResonanceScreenWidget::RebuildWidget()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor_Lambda([this]() { return bBlackout ? FLinearColor::Black : FLinearColor(0.6f, 0.0f, 0.02f, 0.23f); })
		.HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(FMargin(0, 150))
		[
			SNew(SBox).WidthOverride(560)
			.Visibility_Lambda([this]() { return bBlackout ? EVisibility::Hidden : EVisibility::HitTestInvisible; })
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[SNew(STextBlock).Text(FText::FromString(TEXT("공명 폭주"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 30)).ColorAndOpacity(FLinearColor(1, 0.3f, 0.2f))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 12).HAlign(HAlign_Center)
				[SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(FString::Printf(TEXT("남은 시간 %.1f초  |  강인도 %.0f\n강인도 소진으로 폭주 저지"), Remaining, Poise)); }).Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))]
				+ SVerticalBox::Slot().AutoHeight()
				[SNew(SProgressBar).Percent_Lambda([this]() { return FMath::Clamp(Remaining / FMath::Max(0.1f, Duration), 0.0f, 1.0f); }).FillColorAndOpacity(FLinearColor(1, 0.08f, 0.03f))]
			]
		];
}

void UMVResonanceScreenWidget::StartBlackout(float Seconds)
{
	bBlackout = true;
	GetWorld()->GetTimerManager().SetTimer(BlackoutTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		RemoveFromParent();
	}), FMath::Max(0.1f, Seconds), false);
}

void UMVResonanceScreenWidget::NativeDestruct()
{
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(BlackoutTimer);
	Super::NativeDestruct();
}
