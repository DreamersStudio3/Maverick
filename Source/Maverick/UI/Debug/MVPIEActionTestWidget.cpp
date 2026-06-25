#include "UI/Debug/MVPIEActionTestWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Character/MVCharacterBase.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/MVStatComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Struct/MVHitTypes.h"
#include "UI/System/MVUISubsystem.h"

struct FMVPIEActionTestSpec
{
	const TCHAR* Label = TEXT("");
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;
	float HPDamage = 0.0f;
};

namespace
{
const FMVPIEActionTestSpec& GetPIEActionTestSpec(const int32 Index)
{
	static const FMVPIEActionTestSpec Specs[] =
	{
		{ TEXT("SmallHit F / HP -5"), EMVActionHitReactionType::SmallHit, 5.0f },
		{ TEXT("LargeHit F / HP -10"), EMVActionHitReactionType::LargeHit, 10.0f },
		{ TEXT("KnockDown F / HP -15"), EMVActionHitReactionType::KnockDown, 15.0f },
		{ TEXT("Airborne F / HP -20"), EMVActionHitReactionType::Airborne, 20.0f },
	};

	return Specs[FMath::Clamp(Index, 0, UE_ARRAY_COUNT(Specs) - 1)];
}

UTextBlock* PIEActionTestMakeText(
	UWidgetTree& WidgetTree,
	const FName Name,
	const FString& Text,
	const float FontSize,
	const FLinearColor& Color = FLinearColor(0.86f, 0.82f, 0.72f, 1.0f))
{
	UTextBlock* TextBlock = WidgetTree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(Text));
		TextBlock->SetAutoWrapText(true);
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = FontSize;
		TextBlock->SetFont(FontInfo);
	}
	return TextBlock;
}

UButton* PIEActionTestAddButton(UWidgetTree& WidgetTree, UVerticalBox& ParentBox, const FName Name, const FString& Label)
{
	UButton* Button = WidgetTree.ConstructWidget<UButton>(UButton::StaticClass(), Name);
	UTextBlock* ButtonText = PIEActionTestMakeText(
		WidgetTree,
		FName(*(Name.ToString() + TEXT("_Text"))),
		Label,
		14.0f);
	if (Button && ButtonText)
	{
		Button->SetBackgroundColor(FLinearColor(0.22f, 0.20f, 0.16f, 0.62f));
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		Button->IsFocusable = false;
PRAGMA_ENABLE_DEPRECATION_WARNINGS
		Button->AddChild(ButtonText);
		if (UVerticalBoxSlot* Slot = ParentBox.AddChildToVerticalBox(Button))
		{
			Slot->SetPadding(FMargin(0.0f, 6.0f));
		}
	}
	return Button;
}
}

void UMVPIEActionTestWidget::SetTargetCharacter(AMVCharacterBase* InTargetCharacter)
{
	TargetCharacter = InTargetCharacter;
}

void UMVPIEActionTestWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
	BuildNativeWidgetTree();
}

FReply UMVPIEActionTestWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::Q)
	{
		CloseSideWindow();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMVPIEActionTestWidget::BuildNativeWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PIEActionTestRoot"));
	UBorder* SideWindowBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PIEActionTestSideWindow"));
	UVerticalBox* ButtonBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PIEActionTestButtonBox"));
	if (!RootCanvas || !SideWindowBorder || !ButtonBox)
	{
		return;
	}

	WidgetTree->RootWidget = RootCanvas;
	SideWindowBorder->SetBrushColor(FLinearColor(0.015f, 0.014f, 0.012f, 0.74f));
	SideWindowBorder->SetPadding(FMargin(48.0f, 100.0f, 34.0f, 32.0f));
	SideWindowBorder->SetContent(ButtonBox);

	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(SideWindowBorder))
	{
		PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
		PanelSlot->SetAlignment(FVector2D::ZeroVector);
		PanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 430.0f, 0.0f));
	}

	if (UTextBlock* TitleText = PIEActionTestMakeText(*WidgetTree, TEXT("PIEActionTestTitle"), TEXT("PIE Hit Reaction Test"), 22.0f))
	{
		if (UVerticalBoxSlot* TitleSlot = ButtonBox->AddChildToVerticalBox(TitleText))
		{
			TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 32.0f));
		}
	}

	StatusTextBlock = PIEActionTestMakeText(
		*WidgetTree,
		TEXT("PIEActionTestStatus"),
		TEXT("Ready"),
		13.0f,
		FLinearColor(0.72f, 0.70f, 0.62f, 1.0f));
	if (StatusTextBlock)
	{
		if (UVerticalBoxSlot* StatusSlot = ButtonBox->AddChildToVerticalBox(StatusTextBlock))
		{
			StatusSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 22.0f));
		}
	}

	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestSmallHit"), GetPIEActionTestSpec(0).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleSmallHitClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestLargeHit"), GetPIEActionTestSpec(1).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleLargeHitClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestKnockDown"), GetPIEActionTestSpec(2).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleKnockDownClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestAirborne"), GetPIEActionTestSpec(3).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleAirborneClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestResetStats"), TEXT("Reset HP/ST/MP")))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleResetStatsClicked);
	}

	if (UTextBlock* CloseHintText = PIEActionTestMakeText(
		*WidgetTree,
		TEXT("PIEActionTestCloseHint"),
		TEXT("Esc / Q : Close"),
		13.0f,
		FLinearColor(0.70f, 0.68f, 0.60f, 0.95f)))
	{
		if (UVerticalBoxSlot* CloseHintSlot = ButtonBox->AddChildToVerticalBox(CloseHintText))
		{
			CloseHintSlot->SetPadding(FMargin(0.0f, 28.0f, 0.0f, 0.0f));
		}
	}
}

void UMVPIEActionTestWidget::ExecuteTestByIndex(const int32 TestIndex)
{
	AMVCharacterBase* Character = ResolveTargetCharacter();
	if (!Character)
	{
		SetStatusText(TEXT("No target character."));
		return;
	}

	const FMVPIEActionTestSpec& Spec = GetPIEActionTestSpec(TestIndex);
	HideDialogueWindow();

	FMVResolvedHitData HitData;
	HitData.Victim = Character;
	HitData.VictimCharacterIndexCode = Character->GetCharacterIndexCode();
	HitData.ActionRowName = TEXT("PIE_HitReactionTest");
	HitData.FinalDamage = Spec.HPDamage;
	HitData.DamageMultiplier = 1.0f;
	HitData.HitReactionType = Spec.HitReactionType;
	HitData.HitLocation = Character->GetActorLocation();
	HitData.HitDirection = -Character->GetActorForwardVector();

	const bool bHandled = Character->OnHitResolved(HitData);

	UMVStatComponent* StatComponent = Character->FindComponentByClass<UMVStatComponent>();
	if (StatComponent)
	{
		SetStatusText(FString::Printf(
			TEXT("%s | %s | HP %.0f/%.0f"),
			Spec.Label,
			bHandled ? TEXT("Damaged") : TEXT("Ignored"),
			StatComponent->CurrentHP,
			StatComponent->MaxHP));
		return;
	}

	SetStatusText(FString::Printf(TEXT("%s | No StatComponent"), Spec.Label));
}

void UMVPIEActionTestWidget::SetStatusText(const FString& Message)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(Message));
	}
}

AMVCharacterBase* UMVPIEActionTestWidget::ResolveTargetCharacter() const
{
	if (TargetCharacter)
	{
		return TargetCharacter.Get();
	}

	const APlayerController* PlayerController = GetOwningPlayer();
	return PlayerController
		? Cast<AMVCharacterBase>(PlayerController->GetPawn())
		: nullptr;
}

void UMVPIEActionTestWidget::HideDialogueWindow() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UMVUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (UISubsystem)
	{
		UISubsystem->HideDialogueWindow();
	}
}

void UMVPIEActionTestWidget::CloseSideWindow()
{
	RemoveFromParent();

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}
	if (!PlayerController)
	{
		return;
	}

	PlayerController->SetShowMouseCursor(false);
	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
}

void UMVPIEActionTestWidget::HandleSmallHitClicked()
{
	ExecuteTestByIndex(0);
}

void UMVPIEActionTestWidget::HandleLargeHitClicked()
{
	ExecuteTestByIndex(1);
}

void UMVPIEActionTestWidget::HandleKnockDownClicked()
{
	ExecuteTestByIndex(2);
}

void UMVPIEActionTestWidget::HandleAirborneClicked()
{
	ExecuteTestByIndex(3);
}

void UMVPIEActionTestWidget::HandleResetStatsClicked()
{
	AMVCharacterBase* Character = ResolveTargetCharacter();
	UMVStatComponent* StatComponent = Character ? Character->FindComponentByClass<UMVStatComponent>() : nullptr;
	if (!StatComponent)
	{
		SetStatusText(TEXT("No StatComponent."));
		return;
	}

	StatComponent->SetCurrentHP(StatComponent->MaxHP);
	StatComponent->SetCurrentStamina(StatComponent->MaxStamina);
	StatComponent->SetCurrentMP(StatComponent->MaxMP);
	SetStatusText(TEXT("Stats reset."));
}
