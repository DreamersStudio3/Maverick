#include "UI/System/MVUISubsystem.h"

#include "CommonActivatableWidget.h"
#include "Components/MVStatComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UI/Base/MVHUDWidgetBase.h"
#include "UI/Base/MVPopupBase.h"
#include "UI/Base/MVWindowBase.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/System/MVUISettings.h"
#include "UI/Window/MVDeathOverlayWindow.h"

void UMVUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMVUISubsystem::HandleWorldInit);
}

void UMVUISubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	ClearAllUI();

	Super::Deinitialize();
}

UMVUILayerBase* UMVUISubsystem::GetOrCreateRootLayer(UWorld* InWorld)
{
	for (int32 Index = LayerStack.Num() - 1; Index >= 0; --Index)
	{
		if (IsValid(LayerStack[Index]) && LayerStack[Index]->IsInViewport())
		{
			return LayerStack[Index];
		}
		LayerStack.RemoveAt(Index);
	}

	return PushLayer(InWorld);
}

UMVUILayerBase* UMVUISubsystem::PushLayer(UWorld* InWorld)
{
	UWorld* TargetWorld = InWorld ? InWorld : GetWorld();
	if (!TargetWorld || !TargetWorld->IsGameWorld())
	{
		return nullptr;
	}

	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	TSubclassOf<UMVUILayerBase> LayerClass = UMVUILayerBase::StaticClass();
	if (Settings && Settings->RootLayerClass)
	{
		LayerClass = Settings->RootLayerClass;
	}

	APlayerController* PlayerController = TargetWorld->GetFirstPlayerController();
	UMVUILayerBase* NewLayer = PlayerController
		? CreateWidget<UMVUILayerBase>(PlayerController, LayerClass)
		: CreateWidget<UMVUILayerBase>(TargetWorld, LayerClass);

	if (!NewLayer)
	{
		return nullptr;
	}

	const int32 BaseZOrder = Settings ? Settings->RootLayerZOrder : 0;
	const int32 ZOrderStep = Settings ? Settings->LayerZOrderStep : 100;
	NewLayer->AddToViewport(BaseZOrder + LayerStack.Num() * ZOrderStep);
	NewLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	LayerStack.Add(NewLayer);
	return NewLayer;
}

void UMVUISubsystem::PopLayer()
{
	if (LayerStack.IsEmpty())
	{
		return;
	}

	if (UMVUILayerBase* TopLayer = LayerStack.Pop())
	{
		TopLayer->ClearLayer();
		TopLayer->RemoveFromParent();
	}

	CachedHUD = nullptr;
}

UCommonActivatableWidget* UMVUISubsystem::PushWindowByClass(TSubclassOf<UMVWindowBase> WindowClass)
{
	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer || !WindowClass)
	{
		return nullptr;
	}

	return Layer->PushWindow(WindowClass);
}

UCommonActivatableWidget* UMVUISubsystem::PushPopupByClass(TSubclassOf<UMVPopupBase> PopupClass)
{
	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer || !PopupClass)
	{
		return nullptr;
	}

	return Layer->PushPopup(PopupClass);
}

UMVHUDWidgetBase* UMVUISubsystem::ShowHUDByClass(TSubclassOf<UMVHUDWidgetBase> HUDClass)
{
	UMVUILayerBase* Layer = GetOrCreateRootLayer();
	if (!Layer || !HUDClass)
	{
		return nullptr;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	CachedHUD = Cast<UMVHUDWidgetBase>(Layer->SetHUDByClass(HUDClass, PlayerController));
	if (CachedHUD)
	{
		CachedHUD->RefreshHUD();
	}

	return CachedHUD;
}

UMVHUDWidgetBase* UMVUISubsystem::ShowDefaultHUD()
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	return Settings ? ShowHUDByClass(Settings->DefaultHUDClass) : nullptr;
}

UCommonActivatableWidget* UMVUISubsystem::ShowLoadingWindow()
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	return Settings ? PushWindowByClass(Settings->LoadingWindowClass) : nullptr;
}

UCommonActivatableWidget* UMVUISubsystem::ShowDeathOverlay()
{
	const UMVUISettings* Settings = GetDefault<UMVUISettings>();
	if (!Settings)
	{
		return nullptr;
	}

	UCommonActivatableWidget* DeathWindow = PushWindowByClass(Settings->DeathOverlayWindowClass);
	if (UMVDeathOverlayWindow* DeathOverlay = Cast<UMVDeathOverlayWindow>(DeathWindow))
	{
		DeathOverlay->SetDisplaySeconds(Settings->DeathOverlayDuration);
	}

	return DeathWindow;
}

void UMVUISubsystem::ClearAllUI()
{
	for (UMVUILayerBase* Layer : LayerStack)
	{
		if (!Layer)
		{
			continue;
		}

		Layer->ClearLayer();
		Layer->RemoveFromParent();
	}

	LayerStack.Reset();
	CachedHUD = nullptr;
}

void UMVUISubsystem::HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	ClearAllUI();
	GetOrCreateRootLayer(World);

	FTimerHandle PostWorldInitTimerHandle;
	const TWeakObjectPtr<UWorld> WeakWorld = World;
	World->GetTimerManager().SetTimer(
		PostWorldInitTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
		{
			UWorld* CurrentWorld = WeakWorld.Get();
			if (!CurrentWorld || !CurrentWorld->IsGameWorld())
			{
				return;
			}

			ShowDefaultHUD();
			BindToPlayerDeath(CurrentWorld);
		}),
		0.1f,
		false);
}

void UMVUISubsystem::BindToPlayerDeath(UWorld* World)
{
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	UMVStatComponent* StatComponent = Pawn ? Pawn->FindComponentByClass<UMVStatComponent>() : nullptr;
	if (!StatComponent)
	{
		return;
	}

	StatComponent->OnDead.RemoveDynamic(this, &UMVUISubsystem::HandlePlayerDeath);
	StatComponent->OnDead.AddDynamic(this, &UMVUISubsystem::HandlePlayerDeath);
}

void UMVUISubsystem::HandlePlayerDeath()
{
	ShowDeathOverlay();
}
