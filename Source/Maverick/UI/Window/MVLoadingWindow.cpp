#include "UI/Window/MVLoadingWindow.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputCoreTypes.h"
#include "Tables/MVTableManager.h"
#include "UI/System/MVUISettings.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVLoadingWindow, Log, All);

namespace
{
const FText MVLoadingWindowDefaultStepText = NSLOCTEXT("MaverickLoading", "DefaultStepText", "Loading...");

UTextBlock* MVLoadingWindowMakeText(
	UWidgetTree& WidgetTree,
	const FName Name,
	const FText& Text,
	const float FontSize,
	const FLinearColor& Color,
	const ETextJustify::Type Justification = ETextJustify::Left)
{
	UTextBlock* TextBlock = WidgetTree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	if (!TextBlock)
	{
		return nullptr;
	}

	TextBlock->SetText(Text);
	TextBlock->SetAutoWrapText(true);
	TextBlock->SetJustification(Justification);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));

	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	return TextBlock;
}

void MVLoadingWindowAddVerticalChild(UVerticalBox& ParentBox, UWidget* ChildWidget, const FMargin& Padding)
{
	if (!ChildWidget)
	{
		return;
	}

	if (UVerticalBoxSlot* Slot = ParentBox.AddChildToVerticalBox(ChildWidget))
	{
		Slot->SetPadding(Padding);
	}
}
}

UMVLoadingWindow::UMVLoadingWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCloseOnBack = false;

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractInputAction(
		TEXT("/Game/Input/Actions/IA_Interact.IA_Interact"));
	if (InteractInputAction.Succeeded())
	{
		LoadingGuideAdvanceInputAction = InteractInputAction.Object;
	}
}

void UMVLoadingWindow::SetLoadingProgress(const float InProgress, FText InStepText)
{
	LoadingProgress = FMath::Clamp(InProgress, 0.0f, 1.0f);
	LoadingStepText = MoveTemp(InStepText);
	RefreshLoadingProgress();
	OnLoadingProgressChanged.Broadcast(LoadingProgress, LoadingStepText);
}

void UMVLoadingWindow::LoadLoadingGuideCards()
{
	LoadingGuideCards.Reset();
	LoadingGuideCardIndex = INDEX_NONE;

	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings || Settings->GameGuideTableName.IsNone())
	{
		UE_LOG(LogMVLoadingWindow, Warning, TEXT("[LoadingGuide] GameGuide table name is not configured."));
		RefreshLoadingGuideCard();
		return;
	}

	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogMVLoadingWindow, Warning, TEXT("[LoadingGuide] TableManager is not available."));
		RefreshLoadingGuideCard();
		return;
	}

	const FName TableName = Settings->GameGuideTableName;
	const TArray<FString> RowKeys = TableManager->GetRowKeys(TableName);
	if (RowKeys.IsEmpty())
	{
		UE_LOG(LogMVLoadingWindow, Warning, TEXT("[LoadingGuide] No rows found in table '%s'."), *TableName.ToString());
	}

	for (const FString& RowKey : RowKeys)
	{
		bool bEnabled = true;
		TableManager->GetBool(TableName, RowKey, TEXT("bEnabled"), bEnabled);

		bool bShowInLoading = false;
		if (!TableManager->GetBool(TableName, RowKey, TEXT("bShowInLoading"), bShowInLoading))
		{
			UE_LOG(LogMVLoadingWindow, Warning, TEXT("[LoadingGuide] Row '%s' is missing bShowInLoading."), *RowKey);
			continue;
		}

		if (!bEnabled || !bShowInLoading)
		{
			continue;
		}

		FString TitleString;
		FString BodyString;
		if (!TableManager->GetString(TableName, RowKey, TEXT("Title"), TitleString)
			|| !TableManager->GetString(TableName, RowKey, TEXT("Body"), BodyString))
		{
			UE_LOG(LogMVLoadingWindow, Warning, TEXT("[LoadingGuide] Row '%s' is missing Title or Body."), *RowKey);
			continue;
		}

		FString GuideId;
		TableManager->GetString(TableName, RowKey, TEXT("GuideId"), GuideId);

		int32 CardPriority = 0;
		TableManager->GetInt(TableName, RowKey, TEXT("Priority"), CardPriority);

		FMVLoadingGuideCard Card;
		Card.GuideId = GuideId.IsEmpty() ? RowKey : GuideId;
		Card.Title = FText::FromString(TitleString);
		Card.Body = FText::FromString(BodyString);
		Card.Priority = CardPriority;
		LoadingGuideCards.Add(MoveTemp(Card));
	}

	LoadingGuideCards.Sort([](const FMVLoadingGuideCard& Left, const FMVLoadingGuideCard& Right)
	{
		if (Left.Priority != Right.Priority)
		{
			return Left.Priority > Right.Priority;
		}

		return Left.GuideId < Right.GuideId;
	});

	if (!LoadingGuideCards.IsEmpty())
	{
		LoadingGuideCardIndex = 0;
	}
	else
	{
		UE_LOG(LogMVLoadingWindow, Warning, TEXT("[LoadingGuide] No loading guide cards were accepted from '%s'."), *TableName.ToString());
	}

	RefreshLoadingGuideCard();
}

bool UMVLoadingWindow::AdvanceLoadingGuideCard()
{
	if (LoadingGuideCards.Num() <= 1)
	{
		return false;
	}

	if (bGuideCardTransitionInProgress)
	{
		bAdvanceGuideCardAfterTransition = true;
		return true;
	}

	const int32 NewGuideCardIndex = (LoadingGuideCardIndex + 1 + LoadingGuideCards.Num()) % LoadingGuideCards.Num();
	StartLoadingGuideCardTransition(NewGuideCardIndex);
	return true;
}

FMVLoadingGuideCard UMVLoadingWindow::GetCurrentLoadingGuideCard() const
{
	return LoadingGuideCards.IsValidIndex(LoadingGuideCardIndex)
		? LoadingGuideCards[LoadingGuideCardIndex]
		: FMVLoadingGuideCard();
}

void UMVLoadingWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
	BuildNativeWidgetTree();
	LoadLoadingGuideCards();
	RefreshLoadingProgress();
}

void UMVLoadingWindow::NativeOnActivated()
{
	StopLoadingGuideCardTransition();
	LoadLoadingGuideCards();
	RefreshLoadingProgress();

	Super::NativeOnActivated();

	SetKeyboardFocus();
}

void UMVLoadingWindow::NativeOnDeactivated()
{
	StopLoadingGuideCardTransition();

	Super::NativeOnDeactivated();
}

FReply UMVLoadingWindow::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return TryAdvanceLoadingGuideCardFromInput(InKeyEvent.GetKey())
		? FReply::Handled()
		: Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UMVLoadingWindow::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return TryAdvanceLoadingGuideCardFromInput(InKeyEvent.GetKey())
		? FReply::Handled()
		: Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMVLoadingWindow::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LoadingRoot"));
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoadingBackground"));
	USizeBox* CardSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("LoadingGuideCardSize"));
	UBorder* CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoadingGuideCard"));
	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LoadingContent"));
	if (!RootOverlay || !Background || !CardSizeBox || !CardBorder || !ContentBox)
	{
		return;
	}

	WidgetTree->RootWidget = RootOverlay;

	Background->SetBrushColor(FLinearColor(0.012f, 0.012f, 0.014f, 0.96f));
	if (UOverlaySlot* BackgroundSlot = RootOverlay->AddChildToOverlay(Background))
	{
		BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
		BackgroundSlot->SetVerticalAlignment(VAlign_Fill);
	}

	CardSizeBox->SetMaxDesiredWidth(760.0f);
	CardBorder->SetBrushColor(FLinearColor(0.045f, 0.038f, 0.032f, 0.90f));
	CardBorder->SetPadding(FMargin(42.0f, 36.0f));
	CardBorder->SetContent(ContentBox);
	CardSizeBox->SetContent(CardBorder);

	if (UOverlaySlot* CardSlot = RootOverlay->AddChildToOverlay(CardSizeBox))
	{
		CardSlot->SetHorizontalAlignment(HAlign_Center);
		CardSlot->SetVerticalAlignment(VAlign_Center);
		CardSlot->SetPadding(FMargin(48.0f));
	}

	GuideTitleTextBlock = MVLoadingWindowMakeText(
		*WidgetTree,
		TEXT("GuideTitleTextBlock"),
		MVLoadingWindowDefaultStepText,
		30.0f,
		FLinearColor(0.92f, 0.88f, 0.78f, 1.0f));
	MVLoadingWindowAddVerticalChild(*ContentBox, GuideTitleTextBlock, FMargin(0.0f, 0.0f, 0.0f, 18.0f));

	GuideBodyTextBlock = MVLoadingWindowMakeText(
		*WidgetTree,
		TEXT("GuideBodyTextBlock"),
		FText::GetEmpty(),
		17.0f,
		FLinearColor(0.76f, 0.74f, 0.68f, 1.0f));
	if (GuideBodyTextBlock)
	{
		GuideBodyTextBlock->SetWrapTextAt(650.0f);
	}
	MVLoadingWindowAddVerticalChild(*ContentBox, GuideBodyTextBlock, FMargin(0.0f, 0.0f, 0.0f, 28.0f));

	LoadingProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("LoadingProgressBar"));
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetFillColorAndOpacity(FLinearColor(0.82f, 0.19f, 0.14f, 1.0f));
	}
	MVLoadingWindowAddVerticalChild(*ContentBox, LoadingProgressBar, FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	LoadingStepTextBlock = MVLoadingWindowMakeText(
		*WidgetTree,
		TEXT("LoadingStepTextBlock"),
		MVLoadingWindowDefaultStepText,
		13.0f,
		FLinearColor(0.66f, 0.65f, 0.60f, 1.0f),
		ETextJustify::Right);
	MVLoadingWindowAddVerticalChild(*ContentBox, LoadingStepTextBlock, FMargin(0.0f));
}

void UMVLoadingWindow::RefreshLoadingProgress()
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(LoadingProgress);
	}

	if (LoadingStepTextBlock)
	{
		LoadingStepTextBlock->SetText(LoadingStepText.IsEmpty() ? MVLoadingWindowDefaultStepText : LoadingStepText);
	}
}

void UMVLoadingWindow::RefreshLoadingGuideCard()
{
	const bool bHasGuideCard = LoadingGuideCards.IsValidIndex(LoadingGuideCardIndex);
	if (!bHasGuideCard)
	{
		if (GuideTitleTextBlock)
		{
			GuideTitleTextBlock->SetText(MVLoadingWindowDefaultStepText);
		}
		if (GuideBodyTextBlock)
		{
			GuideBodyTextBlock->SetText(FText::GetEmpty());
			GuideBodyTextBlock->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const FMVLoadingGuideCard& Card = LoadingGuideCards[LoadingGuideCardIndex];
	if (GuideTitleTextBlock)
	{
		GuideTitleTextBlock->SetRenderOpacity(1.0f);
		GuideTitleTextBlock->SetText(Card.Title);
	}
	if (GuideBodyTextBlock)
	{
		GuideBodyTextBlock->SetRenderOpacity(1.0f);
		GuideBodyTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
		GuideBodyTextBlock->SetText(Card.Body);
	}
}

bool UMVLoadingWindow::TryAdvanceLoadingGuideCardFromInput(const FKey& Key)
{
	return IsLoadingGuideAdvanceKey(Key) && AdvanceLoadingGuideCard();
}

bool UMVLoadingWindow::IsLoadingGuideAdvanceKey(const FKey& Key) const
{
	if (!Key.IsValid())
	{
		return false;
	}

	return IsMappedLoadingGuideAdvanceActionKey(Key)
		|| (LoadingGuideAdvanceFallbackKey.IsValid() && Key == LoadingGuideAdvanceFallbackKey)
		|| Key == EKeys::Gamepad_FaceButton_Bottom;
}

bool UMVLoadingWindow::IsMappedLoadingGuideAdvanceActionKey(const FKey& Key) const
{
	const UInputAction* AdvanceInputAction = LoadingGuideAdvanceInputAction.LoadSynchronous();
	if (!AdvanceInputAction)
	{
		return false;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()
		: nullptr;
	if (!EnhancedInputSubsystem)
	{
		return false;
	}

	return EnhancedInputSubsystem->QueryKeysMappedToAction(AdvanceInputAction).Contains(Key);
}

void UMVLoadingWindow::StartLoadingGuideCardTransition(const int32 NewGuideCardIndex)
{
	if (!LoadingGuideCards.IsValidIndex(NewGuideCardIndex))
	{
		return;
	}

	PendingLoadingGuideCardIndex = NewGuideCardIndex;
	bGuideCardTransitionInProgress = true;
	bAdvanceGuideCardAfterTransition = false;

	const float FadeSeconds = FMath::Max(0.0f, GuideCardTransitionFadeSeconds);
	if (FadeSeconds <= 0.0f || (!GuideTitleTextBlock && !GuideBodyTextBlock))
	{
		HandleLoadingGuideCardFadeOutFinished();
		return;
	}

	if (GuideTitleTextBlock)
	{
		GuideTitleFadeController.Play(
			*GuideTitleTextBlock,
			GuideTitleTextBlock->GetRenderOpacity(),
			0.0f,
			FadeSeconds,
			true,
			[this]()
			{
				HandleLoadingGuideCardFadeOutFinished();
			});
	}

	if (GuideBodyTextBlock)
	{
		if (GuideTitleTextBlock)
		{
			GuideBodyFadeController.Play(
				*GuideBodyTextBlock,
				GuideBodyTextBlock->GetRenderOpacity(),
				0.0f,
				FadeSeconds,
				true);
		}
		else
		{
			GuideBodyFadeController.Play(
				*GuideBodyTextBlock,
				GuideBodyTextBlock->GetRenderOpacity(),
				0.0f,
				FadeSeconds,
				true,
				[this]()
				{
					HandleLoadingGuideCardFadeOutFinished();
				});
		}
	}
}

void UMVLoadingWindow::HandleLoadingGuideCardFadeOutFinished()
{
	if (!bGuideCardTransitionInProgress || !LoadingGuideCards.IsValidIndex(PendingLoadingGuideCardIndex))
	{
		StopLoadingGuideCardTransition();
		return;
	}

	LoadingGuideCardIndex = PendingLoadingGuideCardIndex;
	PendingLoadingGuideCardIndex = INDEX_NONE;
	RefreshLoadingGuideCard();

	const float FadeSeconds = FMath::Max(0.0f, GuideCardTransitionFadeSeconds);
	if (FadeSeconds <= 0.0f || (!GuideTitleTextBlock && !GuideBodyTextBlock))
	{
		HandleLoadingGuideCardFadeInFinished();
		return;
	}

	if (GuideTitleTextBlock)
	{
		GuideTitleFadeController.Play(
			*GuideTitleTextBlock,
			0.0f,
			1.0f,
			FadeSeconds,
			false,
			[this]()
			{
				HandleLoadingGuideCardFadeInFinished();
			});
	}

	if (GuideBodyTextBlock)
	{
		if (GuideTitleTextBlock)
		{
			GuideBodyFadeController.Play(
				*GuideBodyTextBlock,
				0.0f,
				1.0f,
				FadeSeconds,
				false);
		}
		else
		{
			GuideBodyFadeController.Play(
				*GuideBodyTextBlock,
				0.0f,
				1.0f,
				FadeSeconds,
				false,
				[this]()
				{
					HandleLoadingGuideCardFadeInFinished();
				});
		}
	}
}

void UMVLoadingWindow::HandleLoadingGuideCardFadeInFinished()
{
	bGuideCardTransitionInProgress = false;
	if (bAdvanceGuideCardAfterTransition)
	{
		bAdvanceGuideCardAfterTransition = false;
		AdvanceLoadingGuideCard();
	}
}

void UMVLoadingWindow::StopLoadingGuideCardTransition()
{
	GuideTitleFadeController.Stop();
	GuideBodyFadeController.Stop();
	PendingLoadingGuideCardIndex = INDEX_NONE;
	bGuideCardTransitionInProgress = false;
	bAdvanceGuideCardAfterTransition = false;
	if (GuideTitleTextBlock)
	{
		GuideTitleTextBlock->SetRenderOpacity(1.0f);
	}
	if (GuideBodyTextBlock)
	{
		GuideBodyTextBlock->SetRenderOpacity(1.0f);
	}
}
