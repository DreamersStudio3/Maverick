#include "Interaction/MVCheckpointActor.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVInteractableComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "System/MVFieldTransitionSubsystem.h"
#include "System/MVWorldStateSubsystem.h"
#include "Tags/MVGameplayTags.h"
#include "UI/System/MVUISubsystem.h"
#include "UI/Window/MVInteractionMenuWindow.h"

AMVCheckpointActor::AMVCheckpointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(SceneRoot);
	InteractionVolume->SetSphereRadius(160.0f);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Overlap);

	InteractableComponent = CreateDefaultSubobject<UMVInteractableComponent>(TEXT("Interactable"));

	InactivePromptText = NSLOCTEXT("MaverickCheckpoint", "InactivePrompt", "활성화한다");
	ActivePromptText = NSLOCTEXT("MaverickCheckpoint", "ActivePrompt", "체크포인트");
	CheckpointMenuData = MakeDefaultCheckpointMenuData();
}

void AMVCheckpointActor::BeginPlay()
{
	Super::BeginPlay();

	if (InteractableComponent)
	{
		InteractableComponent->OnInteractionRequested.AddUniqueDynamic(
			this,
			&AMVCheckpointActor::HandleInteractionRequested);
	}

	if (UMVWorldStateSubsystem* WorldState = UMVWorldStateSubsystem::Get(this))
	{
		FMVCheckpointSaveData ActivatedCheckpoint;
		if (WorldState->TryGetActivatedCheckpoint(ResolveCheckpointId(), ActivatedCheckpoint))
		{
			bActivated = true;
		}
	}

	RefreshInteractableState();
}

void AMVCheckpointActor::StartActivation(AActor* Interactor)
{
	if (bActivated || bActivating)
	{
		return;
	}

	bActivating = true;
	ActiveInteractor = Interactor;
	BeginCheckpointInteractionSession();
	RefreshInteractableState();
	OnCheckpointActivationStarted.Broadcast(Interactor, this);
}

void AMVCheckpointActor::FinishActivation()
{
	if (!bActivating && bActivated)
	{
		return;
	}

	bActivating = false;
	bActivated = true;
	SaveAsLastCheckpoint(true);
	RefreshInteractableState();
	OnCheckpointActivated.Broadcast(ActiveInteractor.Get(), this);
	EndCheckpointInteractionSession();
	ActiveInteractor = nullptr;
}

void AMVCheckpointActor::OpenCheckpointMenu(AActor* Interactor)
{
	if (!bActivated || bActivating)
	{
		return;
	}

	ActiveInteractor = Interactor;
	BeginCheckpointInteractionSession();

	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		ActiveMenuWindow = UISubsystem->ShowInteractionMenu(BuildCheckpointMenuData(), this);
		if (ActiveMenuWindow)
		{
			ActiveMenuWindow->OnInteractionMenuClosed.AddUniqueDynamic(this, &AMVCheckpointActor::HandleCheckpointMenuClosed);
			ActiveMenuWindow->OnInteractionMenuActionSelected.AddUniqueDynamic(
				this,
				&AMVCheckpointActor::HandleCheckpointMenuActionSelected);
			return;
		}
	}

	EndCheckpointInteractionSession();
	ActiveInteractor = nullptr;
}

void AMVCheckpointActor::RestAtCheckpoint(AActor* Interactor)
{
	SaveAsLastCheckpoint(true);

	AMVCharacterBase* Character = Cast<AMVCharacterBase>(Interactor);
	if (!Character)
	{
		if (const UWorld* World = GetWorld())
		{
			const APlayerController* PlayerController = World->GetFirstPlayerController();
			Character = PlayerController ? Cast<AMVCharacterBase>(PlayerController->GetPawn()) : nullptr;
		}
	}

	if (Character && Character->StatComponent)
	{
		Character->StatComponent->ResetDeathState();
		Character->StatComponent->SetCurrentHP(Character->StatComponent->MaxHP);
		Character->StatComponent->SetCurrentStamina(Character->StatComponent->MaxStamina);
		Character->StatComponent->SetCurrentMP(Character->StatComponent->MaxMP);
		Character->StatComponent->SetCurrentGroggy(0.0f);
	}
}

void AMVCheckpointActor::RefreshInteractableState()
{
	if (!InteractableComponent)
	{
		return;
	}

	InteractableComponent->SetCanInteract(!bActivating);
	InteractableComponent->SetPromptText(bActivated ? ActivePromptText : InactivePromptText);
}

FMVInteractionMenuData AMVCheckpointActor::MakeDefaultCheckpointMenuData()
{
	FMVInteractionMenuData MenuData;
	MenuData.Title = NSLOCTEXT("MaverickCheckpoint", "MenuTitle", "체크포인트");

	FMVMenuEntryData RestEntry;
	RestEntry.EntryId = MVGameplayTags::Interaction_Menu_Entry_Checkpoint_Rest;
	RestEntry.Label = NSLOCTEXT("MaverickCheckpoint", "Rest", "휴식한다");
	RestEntry.ActionName = TEXT("Rest");
	MenuData.Entries.Add(RestEntry);

	FMVMenuEntryData TravelEntry;
	TravelEntry.EntryId = MVGameplayTags::Interaction_Menu_Entry_Checkpoint_Travel;
	TravelEntry.Label = NSLOCTEXT("MaverickCheckpoint", "Travel", "다른 체크포인트로 이동한다");
	TravelEntry.SubMenuId = MVGameplayTags::Interaction_Menu_Checkpoint_Travel;
	TravelEntry.bCloseMenuOnExecute = false;
	MenuData.Entries.Add(TravelEntry);

	FMVMenuEntryData LevelUpEntry;
	LevelUpEntry.EntryId = MVGameplayTags::Interaction_Menu_Entry_Checkpoint_LevelUp;
	LevelUpEntry.Label = NSLOCTEXT("MaverickCheckpoint", "LevelUp", "레벨 업을 한다");
	LevelUpEntry.ActionName = TEXT("LevelUp");
	LevelUpEntry.bEnabled = false;
	MenuData.Entries.Add(LevelUpEntry);

	FMVMenuEntryData StorageEntry;
	StorageEntry.EntryId = MVGameplayTags::Interaction_Menu_Entry_Checkpoint_Storage;
	StorageEntry.Label = NSLOCTEXT("MaverickCheckpoint", "Storage", "보관함을 이용한다");
	StorageEntry.ActionName = TEXT("Storage");
	StorageEntry.bEnabled = false;
	MenuData.Entries.Add(StorageEntry);

	return MenuData;
}

FMVInteractionMenuData AMVCheckpointActor::BuildCheckpointMenuData()
{
	FMVInteractionMenuData MenuData = CheckpointMenuData.Entries.IsEmpty()
		? MakeDefaultCheckpointMenuData()
		: CheckpointMenuData;
	PendingTravelTargets.Reset();

	FGameplayTag TravelMenuId = MVGameplayTags::Interaction_Menu_Checkpoint_Travel;
	bool bHasTravelEntry = false;
	for (FMVMenuEntryData& Entry : MenuData.Entries)
	{
		if (Entry.EntryId == MVGameplayTags::Interaction_Menu_Entry_Checkpoint_Travel
			|| Entry.ActionName == TEXT("Travel"))
		{
			if (!Entry.EntryId.IsValid())
			{
				Entry.EntryId = MVGameplayTags::Interaction_Menu_Entry_Checkpoint_Travel;
			}
			if (Entry.SubMenuId.IsValid())
			{
				TravelMenuId = Entry.SubMenuId;
			}
			else
			{
				Entry.SubMenuId = TravelMenuId;
			}
			Entry.ActionName = NAME_None;
			Entry.bEnabled = true;
			Entry.bCloseMenuOnExecute = false;
			bHasTravelEntry = true;
			break;
		}
	}

	if (!bHasTravelEntry)
	{
		FMVMenuEntryData TravelEntry;
		TravelEntry.EntryId = MVGameplayTags::Interaction_Menu_Entry_Checkpoint_Travel;
		TravelEntry.Label = NSLOCTEXT("MaverickCheckpoint", "Travel", "다른 체크포인트로 이동한다");
		TravelEntry.SubMenuId = TravelMenuId;
		TravelEntry.bCloseMenuOnExecute = false;
		MenuData.Entries.Add(TravelEntry);
	}

	const FName CurrentCheckpointId = ResolveCheckpointId();
	const FName CurrentMapName = ResolveCurrentMapName();
	TArray<FMVCheckpointSaveData> ActivatedCheckpoints;
	if (const UMVWorldStateSubsystem* WorldState = UMVWorldStateSubsystem::Get(this))
	{
		WorldState->GetActivatedCheckpoints(ActivatedCheckpoints);
	}

	for (const FMVCheckpointSaveData& Checkpoint : ActivatedCheckpoints)
	{
		if (!Checkpoint.bHasCheckpoint
			|| Checkpoint.CheckpointId.IsNone()
			|| Checkpoint.CheckpointId == CurrentCheckpointId)
		{
			continue;
		}

		if (!Checkpoint.MapName.IsNone() && !CurrentMapName.IsNone() && Checkpoint.MapName != CurrentMapName)
		{
			continue;
		}

		const FName ActionName(*FString::Printf(TEXT("TravelTo_%s"), *Checkpoint.CheckpointId.ToString()));
		PendingTravelTargets.Add(ActionName, Checkpoint);

		FMVMenuEntryData TargetEntry;
		TargetEntry.ParentMenuId = TravelMenuId;
		TargetEntry.Label = FText::FromString(Checkpoint.CheckpointId.ToString());
		TargetEntry.ActionName = ActionName;
		TargetEntry.bCloseMenuOnExecute = true;
		MenuData.Entries.Add(TargetEntry);
	}

	if (PendingTravelTargets.IsEmpty())
	{
		FMVMenuEntryData EmptyEntry;
		EmptyEntry.EntryId = MVGameplayTags::Interaction_Menu_Entry_Checkpoint_NoTravelTargets;
		EmptyEntry.ParentMenuId = TravelMenuId;
		EmptyEntry.Label = NSLOCTEXT("MaverickCheckpoint", "NoTravelTargets", "이동 가능한 체크포인트가 없습니다");
		EmptyEntry.bEnabled = false;
		EmptyEntry.bCloseMenuOnExecute = false;
		MenuData.Entries.Add(EmptyEntry);
	}

	return MenuData;
}

void AMVCheckpointActor::BeginCheckpointInteractionSession()
{
	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->BeginInteractionSession(this);
	}
}

void AMVCheckpointActor::EndCheckpointInteractionSession()
{
	if (UMVUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->EndInteractionSession(this);
	}
}

void AMVCheckpointActor::SaveAsLastCheckpoint(const bool bSaveImmediately)
{
	if (UMVFieldTransitionSubsystem* FieldTransition = UMVFieldTransitionSubsystem::Get(this))
	{
		FieldTransition->SetLastCheckpointFromActor(
			this,
			ResolveCheckpointId(),
			FieldId,
			bSaveImmediately);
	}
}

FName AMVCheckpointActor::ResolveCheckpointId() const
{
	return CheckpointId.IsNone() ? GetFName() : CheckpointId;
}

FName AMVCheckpointActor::ResolveCurrentMapName() const
{
	const UWorld* World = GetWorld();
	return World ? FName(*UGameplayStatics::GetCurrentLevelName(World, true)) : NAME_None;
}

UMVUISubsystem* AMVCheckpointActor::GetUISubsystem() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVUISubsystem>() : nullptr;
}

void AMVCheckpointActor::HandleInteractionRequested(AActor* Interactor, UMVInteractableComponent* Interactable)
{
	if (Interactable != InteractableComponent)
	{
		return;
	}

	if (!bActivated)
	{
		StartActivation(Interactor);
		return;
	}

	OpenCheckpointMenu(Interactor);
}

void AMVCheckpointActor::HandleCheckpointMenuClosed(UMVInteractionMenuWindow* MenuWindow)
{
	if (MenuWindow && MenuWindow == ActiveMenuWindow)
	{
		ActiveMenuWindow->OnInteractionMenuClosed.RemoveDynamic(this, &AMVCheckpointActor::HandleCheckpointMenuClosed);
		ActiveMenuWindow->OnInteractionMenuActionSelected.RemoveDynamic(
			this,
			&AMVCheckpointActor::HandleCheckpointMenuActionSelected);
		ActiveMenuWindow = nullptr;
	}

	EndCheckpointInteractionSession();
	ActiveInteractor = nullptr;
}

void AMVCheckpointActor::HandleCheckpointMenuActionSelected(UObject* SourceObject, FName ActionName)
{
	if (SourceObject && SourceObject != this)
	{
		return;
	}

	if (ActionName == TEXT("Rest"))
	{
		RestAtCheckpoint(ActiveInteractor.Get());
	}
	else if (const FMVCheckpointSaveData* TravelTarget = PendingTravelTargets.Find(ActionName))
	{
		if (UMVFieldTransitionSubsystem* FieldTransition = UMVFieldTransitionSubsystem::Get(this))
		{
			FieldTransition->StartCheckpointTravelToTransform(
				TravelTarget->CheckpointId,
				TravelTarget->FieldId,
				TravelTarget->Transform,
				true);
		}
	}

	OnCheckpointMenuActionSelected.Broadcast(this, ActionName);
}
