#include "System/MVWorldStateSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "System/MVWorldSaveGame.h"

namespace
{
const int32 MVWorldStateCurrentSaveVersion = 1;

FString WorldStateResolveFallbackSlotName(const FString& RequestedSlotName, const FString& DefaultSlotName)
{
	if (!RequestedSlotName.IsEmpty())
	{
		return RequestedSlotName;
	}

	return DefaultSlotName.IsEmpty() ? FString(TEXT("MaverickDefault")) : DefaultSlotName;
}
}

void UMVWorldStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ResetSaveData();
	ActiveSaveSlotName = ResolveSlotName(DefaultSaveSlotName);
	ActiveUserIndex = DefaultUserIndex;
}

void UMVWorldStateSubsystem::Deinitialize()
{
	ResetSaveData();

	Super::Deinitialize();
}

UMVWorldStateSubsystem* UMVWorldStateSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVWorldStateSubsystem>() : nullptr;
}

bool UMVWorldStateSubsystem::SaveToSlot(const FString& SlotName, const int32 UserIndex)
{
	const FString ResolvedSlotName = ResolveSlotName(SlotName);
	UMVWorldSaveGame* SaveGame = Cast<UMVWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UMVWorldSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return false;
	}

	CurrentSaveData.SaveVersion = MVWorldStateCurrentSaveVersion;
	CurrentSaveData.SavedAtUtcIso8601 = FDateTime::UtcNow().ToIso8601();
	SaveGame->SaveData = CurrentSaveData;

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, ResolvedSlotName, UserIndex))
	{
		return false;
	}

	ActiveSaveSlotName = ResolvedSlotName;
	ActiveUserIndex = UserIndex;
	bSaveDataDirty = false;
	OnWorldStateSaved.Broadcast(ActiveSaveSlotName);
	return true;
}

bool UMVWorldStateSubsystem::SaveToDefaultSlot()
{
	return SaveToSlot(DefaultSaveSlotName, DefaultUserIndex);
}

bool UMVWorldStateSubsystem::LoadFromSlot(const FString& SlotName, const int32 UserIndex)
{
	const FString ResolvedSlotName = ResolveSlotName(SlotName);
	if (!UGameplayStatics::DoesSaveGameExist(ResolvedSlotName, UserIndex))
	{
		return false;
	}

	USaveGame* LoadedSaveGame = UGameplayStatics::LoadGameFromSlot(ResolvedSlotName, UserIndex);
	const UMVWorldSaveGame* WorldSaveGame = Cast<UMVWorldSaveGame>(LoadedSaveGame);
	if (!WorldSaveGame)
	{
		return false;
	}

	CurrentSaveData = WorldSaveGame->SaveData;
	if (CurrentSaveData.SaveVersion <= 0)
	{
		CurrentSaveData.SaveVersion = MVWorldStateCurrentSaveVersion;
	}

	ActiveSaveSlotName = ResolvedSlotName;
	ActiveUserIndex = UserIndex;
	bSaveDataDirty = false;
	OnWorldStateLoaded.Broadcast(ActiveSaveSlotName);
	OnWorldStateChanged.Broadcast();
	return true;
}

bool UMVWorldStateSubsystem::LoadDefaultSlot()
{
	return LoadFromSlot(DefaultSaveSlotName, DefaultUserIndex);
}

bool UMVWorldStateSubsystem::LoadOrCreateDefaultSlot()
{
	if (DoesSaveSlotExist(DefaultSaveSlotName, DefaultUserIndex))
	{
		return LoadDefaultSlot();
	}

	ResetSaveData();
	return SaveToDefaultSlot();
}

bool UMVWorldStateSubsystem::DeleteSaveSlot(const FString& SlotName, const int32 UserIndex)
{
	const FString ResolvedSlotName = ResolveSlotName(SlotName);
	return UGameplayStatics::DeleteGameInSlot(ResolvedSlotName, UserIndex);
}

bool UMVWorldStateSubsystem::DoesSaveSlotExist(const FString& SlotName, const int32 UserIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(ResolveSlotName(SlotName), UserIndex);
}

void UMVWorldStateSubsystem::ResetSaveData()
{
	CurrentSaveData = FMVWorldSaveData();
	CurrentSaveData.SaveVersion = MVWorldStateCurrentSaveVersion;
	bSaveDataDirty = false;
	OnWorldStateChanged.Broadcast();
}

void UMVWorldStateSubsystem::ApplySaveData(const FMVWorldSaveData& InSaveData)
{
	CurrentSaveData = InSaveData;
	if (CurrentSaveData.SaveVersion <= 0)
	{
		CurrentSaveData.SaveVersion = MVWorldStateCurrentSaveVersion;
	}

	MarkSaveDataDirty();
}

bool UMVWorldStateSubsystem::SetLastCheckpoint(
	const FName CheckpointId,
	const FName FieldId,
	const FTransform& Transform,
	const FName MapName)
{
	if (CheckpointId.IsNone())
	{
		return false;
	}

	CurrentSaveData.LastCheckpoint.bHasCheckpoint = true;
	CurrentSaveData.LastCheckpoint.CheckpointId = CheckpointId;
	CurrentSaveData.LastCheckpoint.FieldId = FieldId;
	CurrentSaveData.LastCheckpoint.MapName = MapName.IsNone() ? ResolveCurrentMapName() : MapName;
	CurrentSaveData.LastCheckpoint.Transform = Transform;
	MarkSaveDataDirty();
	return true;
}

bool UMVWorldStateSubsystem::TryGetLastCheckpoint(FMVCheckpointSaveData& OutCheckpoint) const
{
	OutCheckpoint = CurrentSaveData.LastCheckpoint;
	return CurrentSaveData.LastCheckpoint.bHasCheckpoint;
}

bool UMVWorldStateSubsystem::SetFieldObjectState(
	const FName FieldId,
	const FName ObjectId,
	const bool bConsumed,
	const FName StateId)
{
	if (FieldId.IsNone() || ObjectId.IsNone())
	{
		return false;
	}

	if (FMVFieldObjectSaveData* ExistingRecord = FindFieldObjectRecord(FieldId, ObjectId))
	{
		if (ExistingRecord->bConsumed == bConsumed && ExistingRecord->StateId == StateId)
		{
			return true;
		}

		ExistingRecord->bConsumed = bConsumed;
		ExistingRecord->StateId = StateId;
		MarkSaveDataDirty();
		return true;
	}

	FMVFieldObjectSaveData NewRecord;
	NewRecord.FieldId = FieldId;
	NewRecord.ObjectId = ObjectId;
	NewRecord.bConsumed = bConsumed;
	NewRecord.StateId = StateId;
	CurrentSaveData.FieldObjects.Add(NewRecord);
	MarkSaveDataDirty();
	return true;
}

bool UMVWorldStateSubsystem::MarkOneTimeSpawnConsumed(const FName FieldId, const FName ObjectId)
{
	return SetFieldObjectState(FieldId, ObjectId, true, FName(TEXT("Consumed")));
}

bool UMVWorldStateSubsystem::IsOneTimeSpawnConsumed(const FName FieldId, const FName ObjectId) const
{
	const FMVFieldObjectSaveData* Record = FindFieldObjectRecord(FieldId, ObjectId);
	return Record && Record->bConsumed;
}

bool UMVWorldStateSubsystem::SetWorldFlag(const FName FlagId, const bool bValue)
{
	if (FlagId.IsNone())
	{
		return false;
	}

	if (FMVWorldFlagSaveData* ExistingRecord = FindWorldFlagRecord(FlagId))
	{
		if (ExistingRecord->bValue == bValue)
		{
			return true;
		}

		ExistingRecord->bValue = bValue;
		MarkSaveDataDirty();
		return true;
	}

	FMVWorldFlagSaveData NewRecord;
	NewRecord.FlagId = FlagId;
	NewRecord.bValue = bValue;
	CurrentSaveData.WorldFlags.Add(NewRecord);
	MarkSaveDataDirty();
	return true;
}

bool UMVWorldStateSubsystem::GetWorldFlag(const FName FlagId, const bool bDefaultValue) const
{
	const FMVWorldFlagSaveData* Record = FindWorldFlagRecord(FlagId);
	return Record ? Record->bValue : bDefaultValue;
}

bool UMVWorldStateSubsystem::SetQuestState(const FName QuestId, const FName StateId)
{
	if (QuestId.IsNone())
	{
		return false;
	}

	FMVQuestSaveData& QuestRecord = FindOrAddQuestRecord(QuestId);
	if (QuestRecord.StateId == StateId)
	{
		return true;
	}

	QuestRecord.StateId = StateId;
	MarkSaveDataDirty();
	return true;
}

FName UMVWorldStateSubsystem::GetQuestState(const FName QuestId) const
{
	const FMVQuestSaveData* QuestRecord = FindQuestRecord(QuestId);
	return QuestRecord ? QuestRecord->StateId : NAME_None;
}

bool UMVWorldStateSubsystem::SetQuestObjectiveValue(
	const FName QuestId,
	const FName ObjectiveId,
	const int32 Value)
{
	if (QuestId.IsNone() || ObjectiveId.IsNone())
	{
		return false;
	}

	FMVQuestSaveData& QuestRecord = FindOrAddQuestRecord(QuestId);
	if (const int32* ExistingValue = QuestRecord.ObjectiveValues.Find(ObjectiveId))
	{
		if (*ExistingValue == Value)
		{
			return true;
		}
	}

	QuestRecord.ObjectiveValues.Add(ObjectiveId, Value);
	MarkSaveDataDirty();
	return true;
}

int32 UMVWorldStateSubsystem::GetQuestObjectiveValue(
	const FName QuestId,
	const FName ObjectiveId,
	const int32 DefaultValue) const
{
	const FMVQuestSaveData* QuestRecord = FindQuestRecord(QuestId);
	const int32* Value = QuestRecord ? QuestRecord->ObjectiveValues.Find(ObjectiveId) : nullptr;
	return Value ? *Value : DefaultValue;
}

bool UMVWorldStateSubsystem::CompleteQuest(const FName QuestId)
{
	if (QuestId.IsNone())
	{
		return false;
	}

	FMVQuestSaveData& QuestRecord = FindOrAddQuestRecord(QuestId);
	if (QuestRecord.bCompleted)
	{
		return true;
	}

	QuestRecord.bCompleted = true;
	MarkSaveDataDirty();
	return true;
}

bool UMVWorldStateSubsystem::IsQuestCompleted(const FName QuestId) const
{
	const FMVQuestSaveData* QuestRecord = FindQuestRecord(QuestId);
	return QuestRecord && QuestRecord->bCompleted;
}

void UMVWorldStateSubsystem::MarkSaveDataDirty()
{
	bSaveDataDirty = true;
	OnWorldStateChanged.Broadcast();
}

FString UMVWorldStateSubsystem::ResolveSlotName(const FString& SlotName) const
{
	return WorldStateResolveFallbackSlotName(SlotName, DefaultSaveSlotName);
}

FName UMVWorldStateSubsystem::ResolveCurrentMapName() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return NAME_None;
	}

	return FName(*UGameplayStatics::GetCurrentLevelName(World, true));
}

FMVFieldObjectSaveData* UMVWorldStateSubsystem::FindFieldObjectRecord(const FName FieldId, const FName ObjectId)
{
	return CurrentSaveData.FieldObjects.FindByPredicate([FieldId, ObjectId](const FMVFieldObjectSaveData& Record)
	{
		return Record.FieldId == FieldId && Record.ObjectId == ObjectId;
	});
}

const FMVFieldObjectSaveData* UMVWorldStateSubsystem::FindFieldObjectRecord(
	const FName FieldId,
	const FName ObjectId) const
{
	return CurrentSaveData.FieldObjects.FindByPredicate([FieldId, ObjectId](const FMVFieldObjectSaveData& Record)
	{
		return Record.FieldId == FieldId && Record.ObjectId == ObjectId;
	});
}

FMVWorldFlagSaveData* UMVWorldStateSubsystem::FindWorldFlagRecord(const FName FlagId)
{
	return CurrentSaveData.WorldFlags.FindByPredicate([FlagId](const FMVWorldFlagSaveData& Record)
	{
		return Record.FlagId == FlagId;
	});
}

const FMVWorldFlagSaveData* UMVWorldStateSubsystem::FindWorldFlagRecord(const FName FlagId) const
{
	return CurrentSaveData.WorldFlags.FindByPredicate([FlagId](const FMVWorldFlagSaveData& Record)
	{
		return Record.FlagId == FlagId;
	});
}

FMVQuestSaveData* UMVWorldStateSubsystem::FindQuestRecord(const FName QuestId)
{
	return CurrentSaveData.Quests.FindByPredicate([QuestId](const FMVQuestSaveData& Record)
	{
		return Record.QuestId == QuestId;
	});
}

const FMVQuestSaveData* UMVWorldStateSubsystem::FindQuestRecord(const FName QuestId) const
{
	return CurrentSaveData.Quests.FindByPredicate([QuestId](const FMVQuestSaveData& Record)
	{
		return Record.QuestId == QuestId;
	});
}

FMVQuestSaveData& UMVWorldStateSubsystem::FindOrAddQuestRecord(const FName QuestId)
{
	if (FMVQuestSaveData* ExistingRecord = FindQuestRecord(QuestId))
	{
		return *ExistingRecord;
	}

	FMVQuestSaveData NewRecord;
	NewRecord.QuestId = QuestId;
	return CurrentSaveData.Quests.Add_GetRef(NewRecord);
}
