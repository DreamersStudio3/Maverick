#include "UI/Window/MVInventoryWindow.h"

#include "Blueprint/WidgetTree.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/Button.h"
#include "Components/MVInputManagerComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "ICommonInputModule.h"
#include "Input/CommonUIInputTypes.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "UI/System/MVUISettings.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVInventoryPage.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr int32 MVInventoryFieldMenuPage = 0;
constexpr int32 MVInventoryItemsPage = 1;
constexpr float MVInventoryPageFadeOutSeconds = 0.08f;
constexpr float MVInventoryPageFadeInSeconds = 0.10f;
}

UMVInventoryWindow::UMVInventoryWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	ActivatedVisibility = ESlateVisibility::Visible;
	static const ConstructorHelpers::FClassFinder<UUserWidget> MenuView(TEXT("/Game/UI/Inventory/WBP_InventoryMenu"));
	static const ConstructorHelpers::FClassFinder<UUserWidget> InventoryView(TEXT("/Game/UI/Inventory/WBP_InventoryPage"));
	MenuClass = MenuView.Class;
	InventoryClass = InventoryView.Class;
}

bool UMVInventoryWindow::CanOpenForPlayer(const APlayerController* Controller)
{
	const AMVPlayerCharacter* Player = IsValid(Controller) && Controller->IsLocalController()
		? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
	const UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(Player);
	return IsValid(Player) && Player->StatComponent && !Player->StatComponent->IsDead()
		&& Player->InputManagerComponent && !Player->InputManagerComponent->IsGameplayInputBlocked()
		&& Transition && !Transition->IsTransitionRunning();
}

bool UMVInventoryWindow::HasPresentation() const
{
	return MenuClass && InventoryClass;
}

void UMVInventoryWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	APlayerController* Player = GetOwningPlayer();
	if (!Player || !HasPresentation())
	{
		return;
	}

	UUserWidget* Menu = CreateWidget<UUserWidget>(Player, MenuClass);
	InventoryPage = Cast<UMVInventoryPage>(CreateWidget<UUserWidget>(Player, InventoryClass));
	if (!Menu || !InventoryPage)
	{
		return;
	}
	InventoryButton = Cast<UButton>(Menu->GetWidgetFromName(TEXT("InventoryButton")));
	BackButton = Cast<UButton>(InventoryPage->GetWidgetFromName(TEXT("InventoryBackButton")));
	if (!ensureMsgf(InventoryButton && BackButton, TEXT("Inventory views require InventoryButton and InventoryBackButton.")))
	{
		return;
	}

	Pages = WidgetTree->ConstructWidget<UWidgetSwitcher>();
	WidgetTree->RootWidget = Pages;
	Pages->AddChild(Menu);
	Pages->AddChild(InventoryPage);
	InventoryButton->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowInventoryPage);
	BackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::ReturnToMenu);
}

void UMVInventoryWindow::NativeOnActivated()
{
	if (Pages)
	{
		Pages->SetActiveWidgetIndex(MVInventoryFieldMenuPage);
	}
	Super::NativeOnActivated();

	APlayerController* Controller = GetOwningPlayer();
	if (!Pages || !CanOpenForPlayer(Controller))
	{
		CloseInventory();
		return;
	}

	AMVPlayerCharacter* Player = CastChecked<AMVPlayerCharacter>(Controller->GetPawn());
	ObservedPlayer = Player;
	BlockedInput = Player->InputManagerComponent.Get();
	BlockedInput->BeginGameplayInputBlock();
	Player->OnDamaged.AddUniqueDynamic(this, &ThisClass::HandlePlayerHit);
	Player->StatComponent->OnDead.AddUniqueDynamic(this, &ThisClass::CloseInventory);
	Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePawnChanged);
	UMVFieldTransitionSubsystem::Get(this)->OnFieldTransitionPhaseChanged.AddUniqueDynamic(
		this, &ThisClass::HandleTransitionChanged);

	const FDataTableRowHandle ConfirmAction = ICommonInputModule::GetSettings().GetDefaultClickAction();
	if (!ConfirmAction.IsNull())
	{
		FBindUIActionArgs Args(ConfirmAction, false, FSimpleDelegate::CreateUObject(this, &ThisClass::ConfirmFocusedChoice));
		Args.InputMode = ECommonInputMode::Menu;
		Args.bConsumeInput = true;
		ConfirmInputBinding = RegisterUIActionBinding(Args);
	}

	// 보스 시험 패널 등 HUD 버튼이 메뉴 위에서 클릭을 받지 않도록 한다.
	if (UGameInstance* Instance = GetGameInstance())
	{
		if (UMVUISubsystem* UI = Instance->GetSubsystem<UMVUISubsystem>())
		{
			if (UMVHUDWidgetBase* HUD = UI->GetMainHUD(); HUD && HUD->IsVisible())
			{
				BlockedHUD = HUD;
				PreviousHUDVisibility = HUD->GetVisibility();
				HUD->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
	}
}

void UMVInventoryWindow::NativeOnDeactivated()
{
	ReleaseSession();
	Super::NativeOnDeactivated();
}

void UMVInventoryWindow::NativeDestruct()
{
	ReleaseSession();
	Super::NativeDestruct();
}

void UMVInventoryWindow::ReleaseSession()
{
	PageFade.Stop();
	if (Pages)
	{
		Pages->SetRenderOpacity(1.0f);
		Pages->SetVisibility(ESlateVisibility::Visible);
	}
	if (ConfirmInputBinding.IsValid())
	{
		RemoveActionBinding(ConfirmInputBinding);
		ConfirmInputBinding.Unregister();
		ConfirmInputBinding = {};
	}
	if (InventoryPage)
	{
		InventoryPage->SetInventory(nullptr);
	}
	if (AMVPlayerCharacter* Player = ObservedPlayer.Get())
	{
		Player->OnDamaged.RemoveDynamic(this, &ThisClass::HandlePlayerHit);
		if (Player->StatComponent)
		{
			Player->StatComponent->OnDead.RemoveDynamic(this, &ThisClass::CloseInventory);
		}
	}
	ObservedPlayer.Reset();
	if (APlayerController* Controller = GetOwningPlayer())
	{
		Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePawnChanged);
	}
	if (UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(this))
	{
		Transition->OnFieldTransitionPhaseChanged.RemoveDynamic(this, &ThisClass::HandleTransitionChanged);
	}
	if (UMVInputManagerComponent* Input = BlockedInput.Get())
	{
		Input->EndGameplayInputBlock();
	}
	BlockedInput.Reset();
	if (UMVHUDWidgetBase* HUD = BlockedHUD.Get(); HUD && HUD->GetVisibility() == ESlateVisibility::HitTestInvisible)
	{
		HUD->SetVisibility(PreviousHUDVisibility);
	}
	BlockedHUD.Reset();
}

void UMVInventoryWindow::ConfirmFocusedChoice()
{
	APlayerController* Controller = GetOwningPlayer();
	if (!Controller || !Pages || !IsActivated() || IsFadingOut() || PageFade.IsPlaying())
	{
		return;
	}
	if (Pages->GetActiveWidgetIndex() == MVInventoryFieldMenuPage)
	{
		if (InventoryButton->HasUserFocus(Controller))
		{
			ShowInventoryPage();
		}
		return;
	}
	if (BackButton->HasUserFocus(Controller))
	{
		ReturnToMenu();
		return;
	}
	InventoryPage->ConfirmFocusedChoice();
}

void UMVInventoryWindow::CloseInventory()
{
	if (IsActivated())
	{
		DeactivateWidget();
	}
}

void UMVInventoryWindow::HandlePlayerHit(const FMVResolvedHitData& Hit)
{
	CloseInventory();
}

void UMVInventoryWindow::HandlePawnChanged(APawn* Previous, APawn* Current)
{
	if (Current != ObservedPlayer.Get())
	{
		CloseInventory();
	}
}

void UMVInventoryWindow::HandleTransitionChanged(const EMVFieldTransitionPhase Phase)
{
	if (Phase != EMVFieldTransitionPhase::Idle)
	{
		CloseInventory();
	}
}

UWidget* UMVInventoryWindow::NativeGetDesiredFocusTarget() const
{
	if (!Pages || PageFade.IsPlaying())
	{
		return Super::NativeGetDesiredFocusTarget();
	}
	if (Pages->GetActiveWidgetIndex() == MVInventoryFieldMenuPage)
	{
		return InventoryButton.Get();
	}
	if (UWidget* Focus = InventoryPage->GetPreferredFocus())
	{
		return Focus;
	}
	return BackButton.Get();
}

bool UMVInventoryWindow::NativeOnHandleBackAction()
{
	if (PageFade.IsPlaying())
	{
		return true;
	}
	if (Pages && Pages->GetActiveWidgetIndex() == MVInventoryItemsPage)
	{
		ReturnToMenu();
		return true;
	}
	return Super::NativeOnHandleBackAction();
}

FReply UMVInventoryWindow::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	const UInputAction* Action = GetDefault<UMVUISettings>()->InventoryMenuAction;
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	const UEnhancedInputLocalPlayerSubsystem* Input = LocalPlayer
		? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (IsActivated() && Action && Input && Input->QueryKeysMappedToAction(Action).Contains(Event.GetKey()))
	{
		if (!Event.IsRepeat())
		{
			CloseInventory();
		}
		return FReply::Handled();
	}
	if (PageFade.IsPlaying())
	{
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(Geometry, Event);
}

FReply UMVInventoryWindow::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
	if (PageFade.IsPlaying())
	{
		return FReply::Handled();
	}
	FReply Reply = Super::NativeOnMouseButtonDown(Geometry, Event);
	if (!Reply.IsEventHandled() && IsActivated())
	{
		if (UWidget* FocusTarget = NativeGetDesiredFocusTarget())
		{
			return FReply::Handled().SetUserFocus(FocusTarget->TakeWidget(), EFocusCause::Mouse);
		}
	}
	return Reply;
}

void UMVInventoryWindow::ShowInventoryPage()
{
	TransitionToPage(MVInventoryItemsPage);
}

void UMVInventoryWindow::ReturnToMenu()
{
	TransitionToPage(MVInventoryFieldMenuPage);
}

void UMVInventoryWindow::TransitionToPage(const int32 PageIndex)
{
	if (!IsActivated() || !Pages || !ObservedPlayer.IsValid() || IsFadingOut()
		|| PageFade.IsPlaying() || Pages->GetActiveWidgetIndex() == PageIndex)
	{
		return;
	}

	// 페이지가 클릭을 받지 않는 동안 창이 초점을 유지해 닫기 키를 계속 받는다.
	Pages->SetVisibility(ESlateVisibility::HitTestInvisible);
	PageFade.Play(*Pages, 1.0f, 0.0f, MVInventoryPageFadeOutSeconds, true, [this, PageIndex]()
	{
		AMVPlayerCharacter* Player = ObservedPlayer.Get();
		if (!Player)
		{
			CloseInventory();
			return;
		}

		InventoryPage->SetInventory(PageIndex == MVInventoryItemsPage ? Player->InventoryComponent.Get() : nullptr);
		Pages->SetActiveWidgetIndex(PageIndex);
		PageFade.Play(*Pages, 0.0f, 1.0f, MVInventoryPageFadeInSeconds, false, [this]()
		{
			Pages->SetVisibility(ESlateVisibility::Visible);
			if (UWidget* Target = NativeGetDesiredFocusTarget())
			{
				Target->SetUserFocus(GetOwningPlayer());
			}
		});
	});
	if (PageFade.IsPlaying())
	{
		SetUserFocus(GetOwningPlayer());
	}
}
