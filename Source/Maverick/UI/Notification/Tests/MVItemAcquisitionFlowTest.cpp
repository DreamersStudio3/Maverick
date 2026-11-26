#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVInventoryComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tables/MVItemTableTypes.h"
#include "Tables/MVTableManager.h"
#include "Tags/MVGameplayTags.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/Notification/MVItemAcquisitionPanel.h"
#include "UI/Popup/MVInteractionPromptPopup.h"
#include "UI/System/MVUILayerBase.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVInventoryWindow.h"
#include "UObject/ObjectKey.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMVItemAcquisitionFlowTest, "Maverick.UI.ItemAcquisition.Flow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace
{
UMVItemAcquisitionPanel* MVItemAcquisitionTestPanel(UMVUISubsystem& UI)
{
	auto* Layer = UI.GetOrCreateRootLayer();
	auto* WidgetLayer = Layer ? Cast<UOverlay>(Layer->GetWidgetFromName(TEXT("WidgetLayer"))) : nullptr;
	if (WidgetLayer)
	{
		for (UWidget* Widget : WidgetLayer->GetAllChildren())
		{
			if (auto* Panel = Cast<UMVItemAcquisitionPanel>(Widget))
			{
				return Panel;
			}
		}
	}
	return nullptr;
}

FString MVItemAcquisitionTestLabel(const UWidget& Entry, const FName Name)
{
	const auto* UserWidget = CastChecked<UUserWidget>(&Entry);
	const auto* Label = CastChecked<UTextBlock>(UserWidget->GetWidgetFromName(Name));
	return Label->GetText().ToString();
}

class FMVCheckAcquisitionFlow final : public IAutomationLatentCommand
{
public:
	FMVCheckAcquisitionFlow(FAutomationTestBase& InTest, const FString& InSlot)
		: Test(InTest), Slot(InSlot)
	{
	}

	bool Update() override;

private:
	bool Begin(UWorld& World, UMVUISubsystem& UI);
	void CheckRestoration(UMVUISubsystem& UI, AMVPlayerCharacter& Player);

	FAutomationTestBase& Test;
	FString Slot;
	TWeakObjectPtr<AMVPlayerCharacter> Player;
	TWeakObjectPtr<UWidget> LastFront;
	TSet<FObjectKey> SeenRows;
	TArray<FString> ExpectedNames;
	double StartedSeconds = 0.0;
	double FrontSeconds = 0.0;
	int32 FrontCount = 0;
	bool bInsertedDuringFade = false;
};

bool FMVCheckAcquisitionFlow::Begin(UWorld& World, UMVUISubsystem& UI)
{
	auto* Controller = World.GetFirstPlayerController();
	auto* Save = World.GetGameInstance()->GetSubsystem<UMVWorldStateSubsystem>();
	UClass* PlayerClass = LoadClass<AMVPlayerCharacter>(nullptr,
		TEXT("/Game/Characters/CharacterLogic/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	if (!Test.TestNotNull(TEXT("Local player controller"), Controller)
		|| !Test.TestNotNull(TEXT("World-state subsystem"), Save)
		|| !Test.TestNotNull(TEXT("Game player class"), PlayerClass))
	{
		return false;
	}

	Save->ActiveSaveSlotName = Slot;
	Save->ResetSaveData();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Player = World.SpawnActor<AMVPlayerCharacter>(PlayerClass, FVector(0, 0, 300), FRotator::ZeroRotator, SpawnParams);
	if (!Test.TestNotNull(TEXT("Spawned player"), Player.Get()))
	{
		return false;
	}
	Player->GetCharacterMovement()->DisableMovement();
	Controller->Possess(Player.Get());

	FMVInteractionPromptData PromptData;
	PromptData.InputKey = EKeys::E;
	PromptData.PromptText = FText::FromString(TEXT("아이템을 얻는다"));
	PromptData.CandidateCount = 1;
	UMVInteractionPromptPopup* Prompt = UI.ShowInteractionPrompt(PromptData);
	Test.TestNotNull(TEXT("Existing interaction prompt is visible"), Prompt);

	const FGameplayTag Material = MVGameplayTags::Item_Material_IronShard;
	const FGameplayTag Potion = MVGameplayTags::Item_HealingPotion_Basic;
	const FGameplayTag Weapon = MVGameplayTags::Item_Weapon_DualSword_TestSword;
	UMVInventoryComponent* Inventory = Player->InventoryComponent;
	const int32 ExistingPotionCount = Inventory->GetItemCount(Potion);
	if (ExistingPotionCount > 0)
	{
		Inventory->TryRemoveItem(Potion, ExistingPotionCount);
	}
	const FGameplayTag Tags[] = {Material, Material, Material, Potion, Weapon, Material};
	const UMVTableManager* Tables = UMVTableManager::Get();
	for (const FGameplayTag Tag : Tags)
	{
		const auto* Definition = Tables->FindInventoryItemDefinition(Tag);
		const auto* Row = Definition ? Definition->Row.GetRow<FMVItemTableRow>(TEXT("Test acquisition")) : nullptr;
		if (!Test.TestNotNull(TEXT("Item presentation data"), Row))
		{
			return false;
		}
		ExpectedNames.Add(Row->DisplayName.ToString());
	}
	const int32 BeforeGrant = Inventory->GetItemCount(Material);
	Controller->ConsoleCommand(TEXT("MV.Inventory.Grant Item.Material.IronShard 3"));
	Test.TestEqual(TEXT("One console grant adds exactly three items"), Inventory->GetItemCount(Material), BeforeGrant + 3);
	Test.TestTrue(TEXT("Acquisition did not close interaction prompt"), Prompt && Prompt->GetParent() && !Prompt->IsClosing());
	UMVItemAcquisitionPanel* Panel = MVItemAcquisitionTestPanel(UI);
	if (!Test.TestNotNull(TEXT("Live acquisition panel is in the widget layer"), Panel))
	{
		return false;
	}
	Test.TestEqual(TEXT("Panel passes input through"), Panel->GetVisibility(), ESlateVisibility::HitTestInvisible);
	const auto* Cards = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionCards")));
	const auto* Feed = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionFeed")));
	const bool bThreeCards = Test.TestEqual(TEXT("Grant three creates three bottom cards"), Cards->GetChildrenCount(), 3);
	const bool bThreeRows = Test.TestEqual(TEXT("Grant three creates three rows"), Feed->GetChildrenCount(), 3);
	if (!bThreeCards || !bThreeRows)
	{
		return false;
	}
	for (int32 Index = 0; Index < 3; ++Index)
	{
		Test.TestEqual(TEXT("Each initial card shows the granted item"), MVItemAcquisitionTestLabel(*Cards->GetChildAt(Index), TEXT("ItemNameText")), ExpectedNames[Index]);
		Test.TestEqual(TEXT("NEW appears only on the first unit"), MVItemAcquisitionTestLabel(*Cards->GetChildAt(Index), TEXT("AcquisitionStatusText")),
			Index == 0 ? FString(TEXT("NEW")) : FString(TEXT("획득")));
	}
	Test.TestTrue(TEXT("Grant queued potion"), Inventory->TryAddItem(Potion));
	Test.TestTrue(TEXT("Grant queued weapon"), Inventory->TryAddItem(Weapon));
	UI.HideInteractionPrompt();
	UMVInventoryWindow* Menu = UI.ShowInventoryWindow();
	Test.TestTrue(TEXT("Inventory menu can open over an active notification"), Menu && UI.HasInventoryWindow());
	StartedSeconds = FPlatformTime::Seconds();
	return true;
}

bool FMVCheckAcquisitionFlow::Update()
{
	UWorld* World = GEditor->PlayWorld;
	UMVUISubsystem* UI = World ? World->GetGameInstance()->GetSubsystem<UMVUISubsystem>() : nullptr;
	if (!Test.TestNotNull(TEXT("PIE UI subsystem"), UI))
	{
		return true;
	}
	if (!Player.IsValid() && !Begin(*World, *UI))
	{
		return true;
	}
	UMVItemAcquisitionPanel* Panel = MVItemAcquisitionTestPanel(*UI);
	if (!Test.TestNotNull(TEXT("Acquisition panel remains attached"), Panel))
	{
		return true;
	}

	const double Now = FPlatformTime::Seconds();
	if (Now - StartedSeconds > 25.0)
	{
		Test.AddError(FString::Printf(TEXT("Timed out after %d cards and %d right-side rows"), FrontCount, SeenRows.Num()));
		return true;
	}
	const auto* Cards = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionCards")));
	const auto* Feed = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionFeed")));
	if (Cards->GetChildrenCount() > 3 || Feed->GetChildrenCount() > 3)
	{
		Test.AddError(TEXT("An acquisition stack exceeded the visible limit"));
		return true;
	}
	UWidget* Front = Cards->GetChildAt(0);
	if (Front && Front != LastFront.Get())
	{
		if (FrontCount > 0)
		{
			Test.TestTrue(TEXT("Previous front card had its full reading time"), Now - FrontSeconds >= 2.0);
		}
		if (Test.TestTrue(TEXT("Expected next front"), ExpectedNames.IsValidIndex(FrontCount)))
		{
			Test.TestEqual(TEXT("Front card in grant order"), MVItemAcquisitionTestLabel(*Front, TEXT("ItemNameText")), ExpectedNames[FrontCount]);
		}
		Test.TestEqual(TEXT("Front card remains on top"), CastChecked<UCanvasPanelSlot>(Front->Slot)->GetZOrder(), 3);
		LastFront = Front;
		FrontSeconds = Now;
		++FrontCount;
	}
	if (Front && !bInsertedDuringFade && FrontCount == 1 && Now - FrontSeconds > 2.0 && Front->GetRenderOpacity() < 0.98f)
	{
		bInsertedDuringFade = Test.TestTrue(TEXT("Grant during outgoing fade"),
			Player->InventoryComponent->TryAddItem(MVGameplayTags::Item_Material_IronShard));
	}

	for (UWidget* Row : Feed->GetAllChildren())
	{
		const FObjectKey Key(Row);
		if (!SeenRows.Contains(Key))
		{
			const int32 Index = SeenRows.Num();
			SeenRows.Add(Key);
			if (Test.TestTrue(TEXT("Expected next feed row"), ExpectedNames.IsValidIndex(Index)))
			{
				Test.TestEqual(TEXT("Right-side name in grant order"), MVItemAcquisitionTestLabel(*Row, TEXT("ItemNameText")), ExpectedNames[Index]);
				const auto* Entry = CastChecked<UUserWidget>(Row);
				Test.TestNull(TEXT("Feed has no quantity label"), Entry->GetWidgetFromName(TEXT("ItemAmountText")));
				const auto* Label = CastChecked<UTextBlock>(Entry->GetWidgetFromName(TEXT("ItemNameText")));
				const auto* Justification = FindFProperty<FByteProperty>(UTextBlock::StaticClass(), TEXT("Justification"));
				Test.TestEqual(TEXT("Feed item name is right aligned"), Justification->GetPropertyValue_InContainer(Label), uint8(ETextJustify::Right));
			}
		}
	}

	if (!Front && Feed->GetChildrenCount() == 0)
	{
		Test.TestTrue(TEXT("Sixth grant arrived during fade"), bInsertedDuringFade);
		Test.TestEqual(TEXT("Every card reached the front once"), FrontCount, 6);
		Test.TestEqual(TEXT("Every feed row appeared once"), SeenRows.Num(), 6);
		CheckRestoration(*UI, *Player.Get());
		return true;
	}
	return false;
}

void FMVCheckAcquisitionFlow::CheckRestoration(UMVUISubsystem& UI, AMVPlayerCharacter& Character)
{
	Character.InventoryComponent->TryAddItem(MVGameplayTags::Item_Material_IronShard, 2);
	UI.ResetToDefaultUI();
	auto* Recreated = MVItemAcquisitionTestPanel(UI);
	Test.TestNotNull(TEXT("Pending acquisition survived layer rebuild"), Recreated);
	if (Recreated)
	{
		const auto* Feed = CastChecked<UPanelWidget>(Recreated->GetWidgetFromName(TEXT("AcquisitionFeed")));
		Test.TestEqual(TEXT("Both pending units were retained"), Feed->GetChildrenCount(), 2);
	}

	auto* Controller = CastChecked<APlayerController>(Character.GetController());
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* Replacement = Character.GetWorld()->SpawnActor<AMVPlayerCharacter>(
		Character.GetClass(), Character.GetActorLocation() + FVector(500, 0, 0), FRotator::ZeroRotator, SpawnParams);
	if (Test.TestNotNull(TEXT("Replacement player"), Replacement))
	{
		Replacement->GetCharacterMovement()->DisableMovement();
		Controller->Possess(Replacement);
		Character.InventoryComponent->TryAddItem(MVGameplayTags::Item_Material_IronShard, 8);
		const FGameplayTag Potion = MVGameplayTags::Item_HealingPotion_Basic;
		const int32 Potions = Replacement->InventoryComponent->GetItemCount(Potion);
		if (Potions > 0)
		{
			Replacement->InventoryComponent->TryRemoveItem(Potion, Potions);
		}
		Test.TestTrue(TEXT("Grant replacement pawn a potion"), Replacement->InventoryComponent->TryAddItem(Potion));
		auto* Panel = MVItemAcquisitionTestPanel(UI);
		if (Test.TestNotNull(TEXT("Replacement player presents acquisitions"), Panel))
		{
			const auto* Feed = CastChecked<UPanelWidget>(Panel->GetWidgetFromName(TEXT("AcquisitionFeed")));
			if (Test.TestEqual(TEXT("Old pawn no longer publishes to the UI"), Feed->GetChildrenCount(), 3))
			{
				Test.TestEqual(TEXT("New pawn potion appears"), MVItemAcquisitionTestLabel(*Feed->GetChildAt(2), TEXT("ItemNameText")), ExpectedNames[3]);
			}
		}
	}

	auto* State = UMVWorldStateSubsystem::Get(&Character);
	Test.TestTrue(TEXT("Write only to another test slot"), State->SaveToSlot(Slot + TEXT("_Other"), 0));
	Test.TestNull(TEXT("Changing slots clears active notifications"), MVItemAcquisitionTestPanel(UI));
	if (Replacement)
	{
		Replacement->InventoryComponent->TryAddItem(MVGameplayTags::Item_Material_IronShard, 10);
		Test.TestNotNull(TEXT("New slot presents new grant"), MVItemAcquisitionTestPanel(UI));
	}
	State->ResetSaveData();
	Test.TestNull(TEXT("Reset clears active notifications"), MVItemAcquisitionTestPanel(UI));
}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FMVDeleteAcquisitionTestSlots, FAutomationTestBase*, Test, FString, Slot);

bool FMVDeleteAcquisitionTestSlots::Update()
{
	for (const FString& Name : {Slot, Slot + TEXT("_Other")})
	{
		if (UGameplayStatics::DoesSaveGameExist(Name, 0))
		{
			Test->TestTrue(TEXT("Delete the test save"), UGameplayStatics::DeleteGameInSlot(Name, 0));
		}
	}
	return true;
}

bool FMVItemAcquisitionFlowTest::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("Failed to compile 'StateTree /Game/Characters/NPC/Enemy/Test/ST_ShootingTarget.ST_ShootingTarget'"),
		EAutomationExpectedErrorFlags::Contains, -1, false);
	AddExpectedError(TEXT("State 'Move': Task 'State Tree Blueprint Task Wrapper': Malformed task, missing instance value."),
		EAutomationExpectedErrorFlags::Contains, -1, false);
	UWorld* Map = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Isolated editor map"), Map))
	{
		return false;
	}
	Map->GetWorldSettings()->DefaultGameMode = AGameModeBase::StaticClass();
	const FString Slot = TEXT("CodexAcquisitionFlow_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FMVCheckAcquisitionFlow(*this, Slot));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FMVDeleteAcquisitionTestSlots(this, Slot));
	return true;
}

#endif
