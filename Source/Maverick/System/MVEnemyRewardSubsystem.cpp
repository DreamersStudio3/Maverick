#include "System/MVEnemyRewardSubsystem.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVInventoryComponent.h"
#include "Components/MVStatComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tables/MVTableManager.h"
#include "Tables/MVWeaponTableTypes.h"

bool UMVEnemyRewardSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMVEnemyRewardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (InWorld.GetNetMode() != NM_Standalone)
	{
		return;
	}
	if (UGameInstance* Instance = InWorld.GetGameInstance())
	{
		Instance->GetOnPawnControllerChanged().AddUniqueDynamic(this, &ThisClass::HandlePawnControllerChanged);
	}
	if (UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this))
	{
		State->OnWorldStateLoaded.AddUniqueDynamic(this, &ThisClass::HandleSlotLoaded);
	}
	if (UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(this))
	{
		Transition->OnFieldTransitionPhaseChanged.AddUniqueDynamic(this, &ThisClass::HandleTransitionPhaseChanged);
	}
	ObserveLocalInventory();
	SchedulePendingRewards();
}

void UMVEnemyRewardSubsystem::Deinitialize()
{
	GetWorld()->GetTimerManager().ClearTimer(RetryTimer);
	if (UGameInstance* Instance = GetWorld()->GetGameInstance())
	{
		Instance->GetOnPawnControllerChanged().RemoveDynamic(this, &ThisClass::HandlePawnControllerChanged);
	}
	if (UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this))
	{
		State->OnWorldStateLoaded.RemoveDynamic(this, &ThisClass::HandleSlotLoaded);
	}
	if (UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(this))
	{
		Transition->OnFieldTransitionPhaseChanged.RemoveDynamic(this, &ThisClass::HandleTransitionPhaseChanged);
	}
	if (UMVInventoryComponent* Inventory = ObservedInventory.Get())
	{
		Inventory->OnInventoryChanged.RemoveDynamic(this, &ThisClass::HandleInventoryChanged);
	}
	ObservedInventory.Reset();
	Super::Deinitialize();
}

void UMVEnemyRewardSubsystem::SubmitReward(const FMVEnemyDropTableRow& Row, const FDataTableRowHandle& Source)
{
	UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
	const UMVTableManager* Tables = UMVTableManager::Get(this);
	if (GetWorld()->GetNetMode() != NM_Standalone || !State || !Tables)
	{
		return;
	}

	FMVPendingEnemyReward Reward;
	for (const FMVGuaranteedEnemyDrop& Item : Row.GuaranteedDrops)
	{
		const FMVInventoryItemDefinition* Definition = Tables->FindInventoryItemDefinition(Item.ItemTag);
		const bool bWeapon = Definition && Definition->Row.DataTable
			&& Definition->Row.DataTable->GetRowStruct() == FMVWeaponTableRow::StaticStruct();
		if (!Definition || Item.Quantity <= 0 || (bWeapon && Item.Quantity != 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy reward %s:%s: invalid item %s x%d."),
				*GetNameSafe(Source.DataTable), *Source.RowName.ToString(), *Item.ItemTag.ToString(), Item.Quantity);
			continue;
		}
		Reward.Items.Add(Item);
	}

	if (!Reward.Items.IsEmpty())
	{
		Reward.DefeatId = FGuid::NewGuid();
		State->QueueEnemyReward(MoveTemp(Reward));
		// 지급 직전 슬롯을 바꾸거나 플레이어가 함께 사망해도 접수한 보상은 이 슬롯에 남는다.
		State->SaveActiveSlot();
		SchedulePendingRewards();
	}
}

UMVInventoryComponent* UMVEnemyRewardSubsystem::ObserveLocalInventory()
{
	const UGameInstance* Instance = GetWorld()->GetGameInstance();
	const APlayerController* Controller = Instance ? Instance->GetFirstLocalPlayerController(GetWorld()) : nullptr;
	const auto* Player = Controller ? Cast<AMVPlayerCharacter>(Controller->GetPawn()) : nullptr;
	UMVInventoryComponent* Inventory = IsValid(Player) ? Player->InventoryComponent.Get() : nullptr;
	if (ObservedInventory != Inventory)
	{
		if (UMVInventoryComponent* Previous = ObservedInventory.Get())
		{
			Previous->OnInventoryChanged.RemoveDynamic(this, &ThisClass::HandleInventoryChanged);
		}
		ObservedInventory = Inventory;
		if (Inventory)
		{
			Inventory->OnInventoryChanged.AddUniqueDynamic(this, &ThisClass::HandleInventoryChanged);
		}
	}
	return Inventory;
}

void UMVEnemyRewardSubsystem::SchedulePendingRewards()
{
	const UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
	if (!RetryTimer.IsValid() && State && !State->GetSaveData().PendingEnemyRewards.IsEmpty())
	{
		RetryTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DeliverPendingRewards);
	}
}

void UMVEnemyRewardSubsystem::DeliverPendingRewards()
{
	RetryTimer.Invalidate();
	UMVWorldStateSubsystem* State = UMVWorldStateSubsystem::Get(this);
	const UMVFieldTransitionSubsystem* Transition = UMVFieldTransitionSubsystem::Get(this);
	if (!State || State->GetSaveData().PendingEnemyRewards.IsEmpty() || (Transition && Transition->IsTransitionRunning()))
	{
		return;
	}

	UMVInventoryComponent* Inventory = ObserveLocalInventory();
	const auto* Player = Inventory ? Cast<AMVPlayerCharacter>(Inventory->GetOwner()) : nullptr;
	if (!Player || !Player->HasActorBegunPlay() || !Player->StatComponent || Player->StatComponent->IsDead()
		|| !Inventory->PrepareForEnemyRewards(*State))
	{
		return;
	}

	TArray<FMVPendingEnemyReward> Remaining = State->GetSaveData().PendingEnemyRewards;
	TArray<FMVItemAcquisition> Acquisitions;
	bool bChanged = false;
	for (FMVPendingEnemyReward& Reward : Remaining)
	{
		for (FMVGuaranteedEnemyDrop& Item : Reward.Items)
		{
			const int32 Previous = Item.Quantity;
			Item.Quantity = Inventory->ApplyEnemyRewardItem(Item.ItemTag, Previous, Acquisitions);
			bChanged |= Item.Quantity != Previous;
		}
		Reward.Items.RemoveAll([](const FMVGuaranteedEnemyDrop& Item) { return Item.Quantity <= 0; });
	}
	if (!bChanged)
	{
		return;
	}
	Remaining.RemoveAll([](const FMVPendingEnemyReward& Reward) { return Reward.Items.IsEmpty(); });

	const uint64 Generation = State->GetSaveDataGeneration();
	State->CommitEnemyRewardGrants(MoveTemp(Remaining), Inventory->BuildSaveData());
	State->SaveActiveSlot();
	for (const FMVItemAcquisition& Acquisition : Acquisitions)
	{
		if (!IsValid(Inventory) || State->GetSaveDataGeneration() != Generation)
		{
			break;
		}
		Inventory->NotifyCommittedAcquisition(Acquisition);
	}
}

void UMVEnemyRewardSubsystem::HandlePawnControllerChanged(APawn* Pawn, AController* Controller)
{
	SchedulePendingRewards();
}

void UMVEnemyRewardSubsystem::HandleSlotLoaded(FString SlotName)
{
	SchedulePendingRewards();
}

void UMVEnemyRewardSubsystem::HandleTransitionPhaseChanged(const EMVFieldTransitionPhase Phase)
{
	if (Phase == EMVFieldTransitionPhase::Idle)
	{
		SchedulePendingRewards();
	}
}

void UMVEnemyRewardSubsystem::HandleInventoryChanged(FGameplayTag ItemTag)
{
	SchedulePendingRewards();
}
