#include "UI/Notification/MVItemAcquisitionNotifications.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVInventoryComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tables/MVItemTableTypes.h"
#include "Tables/MVTableManager.h"
#include "UI/Notification/MVItemAcquisitionPanel.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/System/MVUISettings.h"
#include "UI/System/MVUISubsystem.h"

void UMVItemAcquisitionNotifications::Initialize(UMVUISubsystem& InUI)
{
	UI = &InUI;
	Queues = MakeShared<FMVItemAcquisitionQueues>();
	UGameInstance* GameInstance = UI->GetGameInstance();
	GameInstance->GetOnPawnControllerChanged().AddUniqueDynamic(this, &ThisClass::HandlePawnControllerChanged);
	if (auto* State = GameInstance->GetSubsystem<UMVWorldStateSubsystem>())
	{
		WorldState = State;
		SaveSlotName = State->ActiveSaveSlotName;
		SaveUserIndex = State->ActiveUserIndex;
		State->OnWorldStateLoaded.AddUniqueDynamic(this, &ThisClass::HandleSlotLoaded);
		State->OnWorldStateSaved.AddUniqueDynamic(this, &ThisClass::HandleSlotSaved);
		State->OnWorldStateReset.AddUObject(this, &ThisClass::ResetNotifications);
	}
}

void UMVItemAcquisitionNotifications::Shutdown()
{
	UI->GetGameInstance()->GetOnPawnControllerChanged().RemoveDynamic(this, &ThisClass::HandlePawnControllerChanged);
	if (auto* State = WorldState.Get())
	{
		State->OnWorldStateLoaded.RemoveDynamic(this, &ThisClass::HandleSlotLoaded);
		State->OnWorldStateSaved.RemoveDynamic(this, &ThisClass::HandleSlotSaved);
		State->OnWorldStateReset.RemoveAll(this);
	}
	UnbindInventory();
	ResetNotifications();
	UI = nullptr;
}

void UMVItemAcquisitionNotifications::BindPlayer(AMVPlayerCharacter& Player, APlayerController& Controller)
{
	const ULocalPlayer* LocalPlayer = Controller.GetLocalPlayer();
	if (!LocalPlayer || LocalPlayer != UI->GetGameInstance()->GetFirstGamePlayer()
		|| (LocalController.Get() == &Controller && Inventory.Get() == Player.InventoryComponent))
	{
		return;
	}

	UnbindInventory();
	LocalController = &Controller;
	Inventory = Player.InventoryComponent;
	if (auto* Component = Inventory.Get())
	{
		AcquisitionHandle = Component->OnItemAcquired.AddUObject(this, &ThisClass::HandleAcquired);
	}
	if (UMVUILayerBase* Layer = CurrentLayer.Get())
	{
		AttachLayer(*Layer);
	}
}

void UMVItemAcquisitionNotifications::UnbindInventory()
{
	if (auto* Component = Inventory.Get())
	{
		Component->OnItemAcquired.Remove(AcquisitionHandle);
	}
	AcquisitionHandle.Reset();
	Inventory.Reset();
	LocalController.Reset();
}

void UMVItemAcquisitionNotifications::HandlePawnControllerChanged(APawn* Pawn, AController* Controller)
{
	auto* Player = Cast<AMVPlayerCharacter>(Pawn);
	auto* PlayerController = Cast<APlayerController>(Controller);
	if (Player && PlayerController && PlayerController->GetLocalPlayer())
	{
		BindPlayer(*Player, *PlayerController);
	}
	else if (Inventory.IsValid() && Inventory->GetOwner() == Pawn)
	{
		UnbindInventory();
	}
}

void UMVItemAcquisitionNotifications::AttachLayer(UMVUILayerBase& Layer)
{
	if (CurrentLayer.Get() != &Layer)
	{
		DetachLayer();
		CurrentLayer = &Layer;
	}
	APlayerController* Player = LocalController.Get();
	if (!Player || Player->GetWorld() != Layer.GetWorld() || !Layer.IsInViewport()
		|| (Queues->Cards.IsEmpty() && Queues->Feed.IsEmpty()))
	{
		return;
	}

	if (!Panel || !Panel->GetParent() || Panel->GetOwningPlayer() != Player)
	{
		if (Panel)
		{
			Panel->RemoveFromParent();
		}
		const auto Class = GetDefault<UMVUISettings>()->ItemAcquisitionPanelClass;
		Panel = Class ? CreateWidget<UMVItemAcquisitionPanel>(Player, Class) : nullptr;
		if (Panel)
		{
			Layer.AddWidgetLayerWidget(Panel);
			Panel->Present(Queues.ToSharedRef());
		}
	}
	else
	{
		Panel->RefreshVisibleEntries();
	}
}

void UMVItemAcquisitionNotifications::DetachLayer()
{
	if (Panel)
	{
		Panel->RemoveFromParent();
		Panel = nullptr;
	}
	CurrentLayer.Reset();
}

void UMVItemAcquisitionNotifications::HandleAcquired(const FMVItemAcquisition& Acquisition)
{
	if (Acquisition.GrantedQuantity <= 0)
	{
		return;
	}

	const UMVTableManager* Tables = UMVTableManager::Get();
	const FMVInventoryItemDefinition* Definition = Tables ? Tables->FindInventoryItemDefinition(Acquisition.ItemTag) : nullptr;
	const auto* Row = Definition ? Definition->Row.GetRow<FMVItemTableRow>(TEXT("Item acquisition notification")) : nullptr;
	FMVItemAcquisitionDisplayData Data;
	Data.DisplayName = Row && !Row->DisplayName.IsEmpty() ? Row->DisplayName : FText::FromName(Acquisition.ItemTag.GetTagName());
	Data.Icon = Row ? Row->Icon : TSoftObjectPtr<UTexture2D>();
	Data.Quantity = Acquisition.GrantedQuantity;
	Data.bFirstAcquisition = Acquisition.bFirstAcquisition;
	Queues->Cards.Add(Data);
	Queues->Feed.Add(Data);

	APlayerController* Controller = LocalController.Get();
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	if (World && World->GetGameViewport())
	{
		if (UMVUILayerBase* Layer = UI->GetOrCreateRootLayer(World))
		{
			AttachLayer(*Layer);
		}
	}
}

void UMVItemAcquisitionNotifications::ResetNotifications()
{
	DetachLayer();
	Queues = MakeShared<FMVItemAcquisitionQueues>();
}

void UMVItemAcquisitionNotifications::HandleSlotLoaded(FString SlotName)
{
	SaveSlotName = MoveTemp(SlotName);
	if (const auto* State = WorldState.Get())
	{
		SaveUserIndex = State->ActiveUserIndex;
	}
	ResetNotifications();
}

void UMVItemAcquisitionNotifications::HandleSlotSaved(FString SlotName)
{
	const auto* State = WorldState.Get();
	if (SlotName != SaveSlotName || (State && State->ActiveUserIndex != SaveUserIndex))
	{
		HandleSlotLoaded(MoveTemp(SlotName));
	}
}
