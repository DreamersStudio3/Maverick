#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Character/NPC/Enemy/MVEnemy.h"
#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVInventoryComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/Overlay.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Interaction/ItemPickup/MVItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "System/MVEnemyRewardSubsystem.h"
#include "System/MVFieldEnemySpawnPoint.h"
#include "System/MVWorldSaveGame.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tables/MVTableManager.h"
#include "Tags/MVGameplayTags.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/Notification/MVItemAcquisitionPanel.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/System/MVUISubsystem.h"
#include "UObject/ObjectKey.h"
#include "UObject/UnrealType.h"

namespace
{
const FGameplayTag& MVRewardTestShard()
{
	static const FGameplayTag Tag = MVGameplayTags::Item_Material_IronShard;
	return Tag;
}

class FMVCheckDirectEnemyReward final : public IAutomationLatentCommand
{
public:
	FMVCheckDirectEnemyReward(FAutomationTestBase& InTest, const FString& InSlot)
		: Test(InTest), Slot(InSlot), Acquisitions(MakeShared<TArray<FMVItemAcquisition>>())
	{
	}

	bool Update() override;

private:
	bool Begin(UWorld& World, UMVWorldStateSubsystem& State);
	void CheckDisplay(UWorld& World, int32 Expected);
	UMVItemAcquisitionPanel* FindPanel(UWorld& World) const;
	bool CheckFourUnitDisplay(UWorld& World);
	void CheckSavedSlot(FString Name, int32 Count, int32 Remaining);

	FAutomationTestBase& Test;
	FString Slot;
	TSharedRef<TArray<FMVItemAcquisition>> Acquisitions;
	TWeakObjectPtr<UMVInventoryComponent> Inventory;
	int32 Limit = 0;
	int32 Step = 0;
	double AdvanceAfter = 0.0;
	double DisplayDeadline = 0.0;
	FDateTime DeathTriggeredAtUtc;
	TSet<FObjectKey> SeenCards;
	TSet<FObjectKey> SeenRows;
};

bool FMVCheckDirectEnemyReward::Begin(UWorld& World, UMVWorldStateSubsystem& State)
{
	const UMVTableManager* Tables = UMVTableManager::Get();
	const FMVInventoryItemDefinition* Item = Tables ? Tables->FindInventoryItemDefinition(MVRewardTestShard()) : nullptr;
	auto* Controller = World.GetFirstPlayerController();
	UClass* PlayerClass = LoadClass<AMVPlayerCharacter>(nullptr,
		TEXT("/Game/Characters/CharacterLogic/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	const auto* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Table/Props/DT_EnemyDrops.DT_EnemyDrops"));
	if (!Test.TestNotNull(TEXT("Shard definition"), Item) || !Test.TestNotNull(TEXT("Player controller"), Controller)
		|| !Test.TestNotNull(TEXT("Game player class"), PlayerClass) || !Test.TestNotNull(TEXT("Enemy reward table"), Table))
	{
		return false;
	}
	Limit = Item->StackLimit;
	if (!Test.TestTrue(TEXT("Test reward fits into an empty stack"), Limit >= 4))
	{
		return false;
	}

	State.ResetSaveData();
	if (!Test.TestTrue(TEXT("Create empty test slot"), State.SaveToSlot(Slot + TEXT("_Other"), 0)))
	{
		return false;
	}
	FMVWorldSaveData Data = State.GetSaveDataCopy();
	Data.Inventory.bHasRecord = true;
	Data.Inventory.ItemCounts.Add(MVRewardTestShard(), Limit - 1);
	Data.Inventory.ItemCounts.Add(MVGameplayTags::Item_Weapon_DualSword_TestSword, 1);
	State.ApplySaveData(Data);
	if (!Test.TestTrue(TEXT("Create near-full test slot"), State.SaveToSlot(Slot, 0)))
	{
		return false;
	}

	const FTransform EnemyTransform(FVector(1500, 0, 300));
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* Enemy = World.SpawnActor<AMVEnemy>(AMVEnemy::StaticClass(), EnemyTransform, Spawn);
	auto* Point = World.SpawnActorDeferred<AMVFieldEnemySpawnPoint>(AMVFieldEnemySpawnPoint::StaticClass(),
		EnemyTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Test.TestNotNull(TEXT("Test enemy"), Enemy) || !Test.TestNotNull(TEXT("Test spawn point"), Point))
	{
		return false;
	}
	UClass* PointClass = Point->GetClass();
	FindFProperty<FNameProperty>(PointClass, TEXT("FieldId"))->SetPropertyValue_InContainer(Point, TEXT("RewardTest"));
	FindFProperty<FNameProperty>(PointClass, TEXT("SpawnId"))->SetPropertyValue_InContainer(Point, TEXT("RewardBoss"));
	FindFProperty<FObjectProperty>(PointClass, TEXT("InitialEnemy"))->SetObjectPropertyValue_InContainer(Point, Enemy);
	auto* Row = FindFProperty<FStructProperty>(PointClass, TEXT("EnemyDropRow"));
	FDataTableRowHandle& Handle = *Row->ContainerPtrToValuePtr<FDataTableRowHandle>(Point);
	Handle.DataTable = Table;
	Handle.RowName = TEXT("Boss_Carcass_Trial");
	Point->FinishSpawning(EnemyTransform);

	Enemy->StatComponent->SetCurrentHP(Enemy->StatComponent->MaxHP);
	DeathTriggeredAtUtc = FDateTime::UtcNow();
	Enemy->StatComponent->SetCurrentHP(0.0f);
	FMVDeathContext Duplicate;
	Duplicate.DeadActor = Enemy;
	Enemy->StatComponent->OnDeathStarted.Broadcast(Duplicate);
	CheckSavedSlot(Slot, Limit - 1, 4);
	const auto& Pending = State.GetSaveData().PendingEnemyRewards;
	if (Test.TestEqual(TEXT("Queued once on death"), Pending.Num(), 1))
	{
		const double Delay = (Pending[0].AvailableAtUtc - DeathTriggeredAtUtc).GetTotalSeconds();
		Test.TestTrue(TEXT("Death schedules a 2.5-second grant"), Delay >= 2.5 && Delay <= 3.0);
	}
	Test.TestFalse(TEXT("Direct reward spawns no pickup"), static_cast<bool>(TActorIterator<AMVItemPickup>(&World)));

	if (!Test.TestTrue(TEXT("Switch test slots before delivery"), State.LoadFromSlot(Slot + TEXT("_Other"), 0)))
	{
		return false;
	}
	auto* Player = World.SpawnActor<AMVPlayerCharacter>(
		PlayerClass, FVector(0, 0, 300), FRotator::ZeroRotator, Spawn);
	if (!Test.TestNotNull(TEXT("Local player pawn"), Player))
	{
		return false;
	}
	Player->GetCharacterMovement()->DisableMovement();
	Controller->Possess(Player);
	Inventory = Player->InventoryComponent;
	Inventory->OnItemAcquired.AddLambda([Events = Acquisitions](const FMVItemAcquisition& Acquisition)
	{
		Events->Add(Acquisition);
	});
	return true;
}

UMVItemAcquisitionPanel* FMVCheckDirectEnemyReward::FindPanel(UWorld& World) const
{
	auto* UI = World.GetGameInstance()->GetSubsystem<UMVUISubsystem>();
	auto* Layer = UI->GetOrCreateRootLayer();
	auto* Widgets = Layer ? Cast<UOverlay>(Layer->GetWidgetFromName(TEXT("WidgetLayer"))) : nullptr;
	if (!Widgets)
	{
		return nullptr;
	}
	for (UWidget* Widget : Widgets->GetAllChildren())
	{
		if (auto* Panel = Cast<UMVItemAcquisitionPanel>(Widget))
		{
			return Panel;
		}
	}
	return nullptr;
}

void FMVCheckDirectEnemyReward::CheckDisplay(UWorld& World, const int32 Expected)
{
	auto* Panel = FindPanel(World);
	if (Test.TestNotNull(TEXT("Acquisition panel"), Panel))
	{
		const auto* Cards = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionCards")));
		const auto* Feed = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionFeed")));
		Test.TestEqual(TEXT("Visible cards match granted units"), Cards->GetChildrenCount(), Expected);
		Test.TestEqual(TEXT("Visible rows match granted units"), Feed->GetChildrenCount(), Expected);
	}
}

bool FMVCheckDirectEnemyReward::CheckFourUnitDisplay(UWorld& World)
{
	auto* Panel = FindPanel(World);
	if (!Test.TestNotNull(TEXT("Four-unit acquisition panel"), Panel))
	{
		return true;
	}
	const auto* Cards = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionCards")));
	const auto* Feed = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionFeed")));
	Test.TestTrue(TEXT("Both displays show at most three units"), Cards->GetChildrenCount() <= 3 && Feed->GetChildrenCount() <= 3);
	for (UWidget* Card : Cards->GetAllChildren()) SeenCards.Add(FObjectKey(Card));
	for (UWidget* Row : Feed->GetAllChildren()) SeenRows.Add(FObjectKey(Row));
	if ((SeenCards.Num() >= 4 && SeenRows.Num() >= 4) || FPlatformTime::Seconds() > DisplayDeadline)
	{
		Test.TestEqual(TEXT("Fourth card appears after a space opens"), SeenCards.Num(), 4);
		Test.TestEqual(TEXT("Fourth feed row appears after a space opens"), SeenRows.Num(), 4);
		return true;
	}
	return false;
}

void FMVCheckDirectEnemyReward::CheckSavedSlot(FString Name, const int32 Count, const int32 Remaining)
{
	const auto* Saved = Cast<UMVWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(Name, 0));
	if (Test.TestNotNull(TEXT("Read persisted reward save"), Saved))
	{
		const FMVWorldSaveData& Data = Saved->SaveData;
		Test.TestEqual(TEXT("Saved shard count"), Data.Inventory.ItemCounts.FindRef(MVRewardTestShard()), Count);
		Test.TestEqual(TEXT("At most one record for a defeat"), Data.PendingEnemyRewards.Num(), Remaining > 0 ? 1 : 0);
		if (!Data.PendingEnemyRewards.IsEmpty() && !Data.PendingEnemyRewards[0].Items.IsEmpty())
		{
			Test.TestTrue(TEXT("Saved defeat identity"), Data.PendingEnemyRewards[0].DefeatId.IsValid());
			Test.TestEqual(TEXT("Saved pending quantity"), Data.PendingEnemyRewards[0].Items[0].Quantity, Remaining);
		}
	}
}

bool FMVCheckDirectEnemyReward::Update()
{
	UWorld* World = GEditor->PlayWorld;
	auto* State = UMVWorldStateSubsystem::Get(World);
	if (!Test.TestNotNull(TEXT("PIE world state"), State))
	{
		return true;
	}

	const double Now = FPlatformTime::Seconds();
	if (Now < AdvanceAfter)
	{
		return false;
	}
	AdvanceAfter = Now + 0.1;
	switch (Step++)
	{
	case 0:
		return !Begin(*World, *State);
	case 1:
		Test.TestEqual(TEXT("No reward in the second slot"), Inventory->GetItemCount(MVRewardTestShard()), 0);
		Test.TestEqual(TEXT("No acquisition in the second slot"), Acquisitions->Num(), 0);
		return !Test.TestTrue(TEXT("Reload original test slot"), State->LoadFromSlot(Slot, 0));
	case 2:
		Test.TestEqual(TEXT("Reload preserves the wait before delivery"), Inventory->GetItemCount(MVRewardTestShard()), Limit - 1);
		Test.TestEqual(TEXT("No acquisition during presentation delay"), Acquisitions->Num(), 0);
		Test.TestNull(TEXT("No acquisition panel before the grant"), FindPanel(*World));
		Inventory->OnInventoryChanged.Broadcast(MVRewardTestShard());
		AdvanceAfter = Now + 2.8;
		return false;
	case 3:
		Test.TestEqual(TEXT("Partial grant fills free space"), Inventory->GetItemCount(MVRewardTestShard()), Limit);
		if (Test.TestEqual(TEXT("One partial grant event"), Acquisitions->Num(), 1))
		{
			Test.TestEqual(TEXT("Partial grant quantity"), (*Acquisitions)[0].GrantedQuantity, 1);
		}
		CheckDisplay(*World, 1);
		CheckSavedSlot(Slot, Limit, 3);
		AdvanceAfter = Now + 3.7;
		return false;
	case 4:
		return !Test.TestTrue(TEXT("Make room for pending shards"), Inventory->TryRemoveItem(MVRewardTestShard(), 3));
	case 5:
		if (Test.TestEqual(TEXT("Pending grant event received once"), Acquisitions->Num(), 2))
		{
			Test.TestEqual(TEXT("Pending grant quantity"), (*Acquisitions)[1].GrantedQuantity, 3);
			Test.TestFalse(TEXT("Pending grant is a repeat acquisition"), (*Acquisitions)[1].bFirstAcquisition);
		}
		CheckDisplay(*World, 3);
		CheckSavedSlot(Slot, Limit, 0);
		return !Test.TestTrue(TEXT("Reload after completing grant"), State->LoadFromSlot(Slot, 0));
	case 6:
	{
		const FGameplayTag Weapon = MVGameplayTags::Item_Weapon_DualSword_TestSword;
		Test.TestEqual(TEXT("Owned weapon present"), Inventory->GetItemCount(Weapon), 1);
		Test.TestEqual(TEXT("Reload emits no new acquisition"), Acquisitions->Num(), 2);
		FMVEnemyDropTableRow DuplicateWeapon;
		DuplicateWeapon.GuaranteedDrops.AddDefaulted_GetRef().ItemTag = Weapon;
		World->GetSubsystem<UMVEnemyRewardSubsystem>()->SubmitReward(DuplicateWeapon, FDataTableRowHandle());
		AdvanceAfter = Now + 2.8;
		return false;
	}
	case 7:
	{
		Test.TestEqual(TEXT("Duplicate weapon emits no new acquisition"), Acquisitions->Num(), 2);
		CheckSavedSlot(Slot, Limit, 0);
		CheckSavedSlot(Slot + TEXT("_Other"), 0, 0);
		if (!Test.TestTrue(TEXT("Make room for a complete four-unit grant"), Inventory->TryRemoveItem(MVRewardTestShard(), 4)))
		{
			return true;
		}
		FDataTableRowHandle Handle;
		Handle.DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Table/Props/DT_EnemyDrops.DT_EnemyDrops"));
		Handle.RowName = TEXT("Boss_Carcass_Trial");
		const auto* Row = Handle.GetRow<FMVEnemyDropTableRow>(TEXT("Four-unit reward test"));
		if (!Test.TestNotNull(TEXT("Four-unit reward row"), Row))
		{
			return true;
		}
		World->GetSubsystem<UMVEnemyRewardSubsystem>()->SubmitReward(*Row, Handle);
		return false;
	}
	case 8:
		Test.TestEqual(TEXT("Inventory changes do not release a new reward early"), Inventory->GetItemCount(MVRewardTestShard()), Limit - 4);
		Test.TestEqual(TEXT("No four-unit signal during the delay"), Acquisitions->Num(), 2);
		Test.TestNull(TEXT("No panel during the four-unit reward delay"), FindPanel(*World));
		AdvanceAfter = Now + 2.8;
		return false;
	case 9:
		if (Test.TestEqual(TEXT("Four granted items use one acquisition event"), Acquisitions->Num(), 3))
		{
			Test.TestEqual(TEXT("Complete grant quantity"), (*Acquisitions)[2].GrantedQuantity, 4);
		}
		CheckDisplay(*World, 3);
		DisplayDeadline = Now + 8.0;
		return CheckFourUnitDisplay(*World);
	default:
		return CheckFourUnitDisplay(*World);
	}
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVEnemyRewardTest, "Maverick.Inventory.EnemyReward",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FMVCleanEnemyRewardSlots, FString, Slot);

bool FMVCleanEnemyRewardSlots::Update()
{
	for (const FString& Name : {Slot, Slot + TEXT("_Other")})
	{
		UGameplayStatics::DeleteGameInSlot(Name, 0);
	}
	return true;
}

bool FMVEnemyRewardTest::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("Failed to compile 'StateTree /Game/Characters/NPC/Enemy/Test/ST_ShootingTarget.ST_ShootingTarget'"),
		EAutomationExpectedErrorFlags::Contains, -1, false);
	AddExpectedError(TEXT("State 'Move': Task 'State Tree Blueprint Task Wrapper': Malformed task, missing instance value."),
		EAutomationExpectedErrorFlags::Contains, -1, false);
	UWorld* Map = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Isolated PIE map"), Map))
	{
		return false;
	}
	Map->GetWorldSettings()->DefaultGameMode = AGameModeBase::StaticClass();
	const FString Slot = TEXT("EnemyRewardTest_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FMVCheckDirectEnemyReward(*this, Slot));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FMVCleanEnemyRewardSlots(Slot));
	return true;
}

#endif
