#include "UI/Debug/MVPIEActionTestWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Character/MVCharacterBase.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Combat/MVHitResolverSubsystem.h"
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
	float GroggyDamage = 0.0f;
};

namespace
{
const FMVPIEActionTestSpec& GetPIEActionTestSpec(const int32 Index)
{
	static const FMVPIEActionTestSpec Specs[] =
	{
		{ TEXT("Flinch F / HP -5 / Groggy +10"), EMVActionHitReactionType::Flinch, 5.0f, 10.0f },
		{ TEXT("Stagger F / HP -10 / Groggy +35"), EMVActionHitReactionType::Stagger, 10.0f, 35.0f },
		{ TEXT("Knockback F / HP -12 / Groggy +50"), EMVActionHitReactionType::Knockback, 12.0f, 50.0f },
		{ TEXT("KnockDown F / HP -15 / Groggy +75"), EMVActionHitReactionType::KnockDown, 15.0f, 75.0f },
		{ TEXT("Airborne F / HP -20 / Groggy +0"), EMVActionHitReactionType::Airborne, 20.0f, 0.0f },
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

float PIEActionTestResolveAttackPower(
	const AMVCharacterBase& Attacker,
	const UMVHitResolverSubsystem& HitResolver)
{
	if (const UMVWeaponComponent* WeaponComponent = Attacker.FindComponentByClass<UMVWeaponComponent>())
	{
		const FMVWeaponHitSnapshot WeaponSnapshot = WeaponComponent->CaptureWeaponHitSnapshot();
		if (WeaponSnapshot.bValid && WeaponSnapshot.AttackPower > 0.0f)
		{
			return WeaponSnapshot.AttackPower;
		}
	}

	return FMath::Max(0.0f, HitResolver.FallbackAttackPower);
}

float PIEActionTestMakeDamageMultiplier(const float DesiredDamage, const float AttackPower)
{
	return AttackPower > 0.0f
		? FMath::Max(0.0f, DesiredDamage) / AttackPower
		: 0.0f;
}

EMVHitReactionDirection PIEActionTestDirectionFromIndex(const int32 DirectionIndex)
{
	switch (DirectionIndex)
	{
	case 1:
		return EMVHitReactionDirection::Back;
	case 2:
		return EMVHitReactionDirection::Left;
	case 3:
		return EMVHitReactionDirection::Right;
	case 0:
	default:
		return EMVHitReactionDirection::Front;
	}
}

FString PIEActionTestDirectionToken(const EMVHitReactionDirection Direction)
{
	switch (Direction)
	{
	case EMVHitReactionDirection::Back:
		return TEXT("B");
	case EMVHitReactionDirection::Left:
		return TEXT("L");
	case EMVHitReactionDirection::Right:
		return TEXT("R");
	case EMVHitReactionDirection::Front:
	default:
		return TEXT("F");
	}
}

FVector PIEActionTestResolveHitDirectionForFlinch(
	const AMVCharacterBase& Target,
	const EMVHitReactionDirection Direction)
{
	FVector HitSourceDirection = Target.GetActorForwardVector().GetSafeNormal2D();
	switch (Direction)
	{
	case EMVHitReactionDirection::Back:
		HitSourceDirection = -Target.GetActorForwardVector().GetSafeNormal2D();
		break;
	case EMVHitReactionDirection::Left:
		HitSourceDirection = -Target.GetActorRightVector().GetSafeNormal2D();
		break;
	case EMVHitReactionDirection::Right:
		HitSourceDirection = Target.GetActorRightVector().GetSafeNormal2D();
		break;
	case EMVHitReactionDirection::Front:
	default:
		HitSourceDirection = Target.GetActorForwardVector().GetSafeNormal2D();
		break;
	}

	return HitSourceDirection.IsNearlyZero()
		? FVector::ZeroVector
		: -HitSourceDirection;
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

	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestFlinch"), GetPIEActionTestSpec(0).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleFlinchClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestStagger"), GetPIEActionTestSpec(1).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleStaggerClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestKnockback"), GetPIEActionTestSpec(2).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleKnockbackClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestKnockDown"), GetPIEActionTestSpec(3).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleKnockDownClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestAirborne"), GetPIEActionTestSpec(4).Label))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleAirborneClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestResetStats"), TEXT("Reset HP/ST/MP/Groggy")))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleResetStatsClicked);
	}

	if (UTextBlock* DirectionTitleText = PIEActionTestMakeText(
		*WidgetTree,
		TEXT("PIEActionTestFlinchDirectionTitle"),
		TEXT("Flinch Direction"),
		16.0f,
		FLinearColor(0.86f, 0.82f, 0.72f, 1.0f)))
	{
		if (UVerticalBoxSlot* DirectionTitleSlot = ButtonBox->AddChildToVerticalBox(DirectionTitleText))
		{
			DirectionTitleSlot->SetPadding(FMargin(0.0f, 24.0f, 0.0f, 6.0f));
		}
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestFlinchFront"), TEXT("Flinch F")))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleFlinchFrontClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestFlinchBack"), TEXT("Flinch B")))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleFlinchBackClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestFlinchLeft"), TEXT("Flinch L")))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleFlinchLeftClicked);
	}
	if (UButton* Button = PIEActionTestAddButton(*WidgetTree, *ButtonBox, TEXT("PIEActionTestFlinchRight"), TEXT("Flinch R")))
	{
		Button->OnClicked.AddDynamic(this, &UMVPIEActionTestWidget::HandleFlinchRightClicked);
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
	AMVCharacterBase* Target = ResolveTargetCharacter();
	if (!Target)
	{
		SetStatusText(TEXT("No target character."));
		return;
	}

	AMVCharacterBase* Attacker = ResolveAttackerCharacter();
	if (!Attacker)
	{
		SetStatusText(TEXT("No attacker character."));
		return;
	}

	if (Attacker == Target)
	{
		SetStatusText(TEXT("Target must not be the player attacker."));
		return;
	}

	UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(this);
	if (!HitResolver)
	{
		SetStatusText(TEXT("No HitResolverSubsystem."));
		return;
	}

	const FMVPIEActionTestSpec& Spec = GetPIEActionTestSpec(TestIndex);
	HideDialogueWindow();
	const float TestAttackPower = PIEActionTestResolveAttackPower(*Attacker, *HitResolver);

	FMVHitResolveRequest Request;
	Request.Attacker = Attacker;
	Request.Victim = Target;
	Request.DamageMultiplier = PIEActionTestMakeDamageMultiplier(Spec.HPDamage, TestAttackPower);
	Request.HitReactionType = Spec.HitReactionType;
	Request.GroggyDamageMultiplier = PIEActionTestMakeDamageMultiplier(Spec.GroggyDamage, TestAttackPower);
	Request.HitLocation = Target->GetActorLocation();
	Request.ImpactNormal = Attacker->GetActorLocation() - Target->GetActorLocation();

	FMVResolvedHitData HitData;
	const bool bHandled = HitResolver->ResolveAttackHit(Request, HitData);

	UMVStatComponent* StatComponent = Target->FindComponentByClass<UMVStatComponent>();
	if (StatComponent)
	{
		SetStatusText(FString::Printf(
			TEXT("%s | %s | Damage %.0f | HP %.0f/%.0f | Groggy %.0f/%.0f"),
			Spec.Label,
			bHandled ? TEXT("Damaged") : TEXT("Ignored"),
			HitData.FinalDamage,
			StatComponent->CurrentHP,
			StatComponent->MaxHP,
			StatComponent->CurrentGroggy,
			StatComponent->MaxGroggy));
		return;
	}

	SetStatusText(FString::Printf(TEXT("%s | No StatComponent"), Spec.Label));
}

void UMVPIEActionTestWidget::ExecuteDirectionalFlinchTest(const int32 DirectionIndex)
{
	AMVCharacterBase* Target = ResolveTargetCharacter();
	if (!Target)
	{
		SetStatusText(TEXT("No target character."));
		return;
	}

	AMVCharacterBase* Attacker = ResolveAttackerCharacter();
	if (!Attacker)
	{
		SetStatusText(TEXT("No attacker character."));
		return;
	}

	const EMVHitReactionDirection Direction = PIEActionTestDirectionFromIndex(DirectionIndex);
	const FVector HitDirection = PIEActionTestResolveHitDirectionForFlinch(*Target, Direction);
	if (HitDirection.IsNearlyZero())
	{
		SetStatusText(TEXT("No valid flinch direction."));
		return;
	}

	HideDialogueWindow();

	FMVResolvedHitData HitData;
	HitData.Attacker = Attacker;
	HitData.Victim = Target;
	HitData.AttackerCharacterIndexCode = Attacker->GetCharacterIndexCode();
	HitData.VictimCharacterIndexCode = Target->GetCharacterIndexCode();
	HitData.HitReactionType = EMVActionHitReactionType::Flinch;
	HitData.HitLocation = Target->GetActorLocation();
	HitData.ImpactNormal = -HitDirection;
	HitData.HitDirection = HitDirection;

	const bool bHandled = Target->OnHitResolved(HitData);
	SetStatusText(FString::Printf(
		TEXT("Flinch %s | %s"),
		*PIEActionTestDirectionToken(Direction),
		bHandled ? TEXT("Started") : TEXT("Ignored")));
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

AMVCharacterBase* UMVPIEActionTestWidget::ResolveAttackerCharacter() const
{
	const APlayerController* PlayerController = GetOwningPlayer()
		? GetOwningPlayer()
		: (GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
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

void UMVPIEActionTestWidget::HandleFlinchClicked()
{
	ExecuteTestByIndex(0);
}

void UMVPIEActionTestWidget::HandleStaggerClicked()
{
	ExecuteTestByIndex(1);
}

void UMVPIEActionTestWidget::HandleKnockbackClicked()
{
	ExecuteTestByIndex(2);
}

void UMVPIEActionTestWidget::HandleKnockDownClicked()
{
	ExecuteTestByIndex(3);
}

void UMVPIEActionTestWidget::HandleAirborneClicked()
{
	ExecuteTestByIndex(4);
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
	StatComponent->ResetGroggyState();
	SetStatusText(TEXT("Stats reset. Groggy reset."));
}

void UMVPIEActionTestWidget::HandleFlinchFrontClicked()
{
	ExecuteDirectionalFlinchTest(0);
}

void UMVPIEActionTestWidget::HandleFlinchBackClicked()
{
	ExecuteDirectionalFlinchTest(1);
}

void UMVPIEActionTestWidget::HandleFlinchLeftClicked()
{
	ExecuteDirectionalFlinchTest(2);
}

void UMVPIEActionTestWidget::HandleFlinchRightClicked()
{
	ExecuteDirectionalFlinchTest(3);
}
