#include "System/MVQuestSubsystem.h"

#include "System/MVWorldStateSubsystem.h"

void UMVQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UMVWorldStateSubsystem::StaticClass());

	Super::Initialize(Collection);
}

UMVQuestSubsystem* UMVQuestSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UMVQuestSubsystem>() : nullptr;
}

bool UMVQuestSubsystem::SetQuestState(
	const FName QuestId,
	const FName StateId,
	const bool bSaveImmediately)
{
	UMVWorldStateSubsystem* WorldState = GetWorldState();
	if (!WorldState || !WorldState->SetQuestState(QuestId, StateId))
	{
		return false;
	}

	return SaveDefaultIfRequested(bSaveImmediately);
}

FName UMVQuestSubsystem::GetQuestState(const FName QuestId) const
{
	const UMVWorldStateSubsystem* WorldState = GetWorldState();
	return WorldState ? WorldState->GetQuestState(QuestId) : NAME_None;
}

bool UMVQuestSubsystem::SetQuestObjectiveValue(
	const FName QuestId,
	const FName ObjectiveId,
	const int32 Value,
	const bool bSaveImmediately)
{
	UMVWorldStateSubsystem* WorldState = GetWorldState();
	if (!WorldState || !WorldState->SetQuestObjectiveValue(QuestId, ObjectiveId, Value))
	{
		return false;
	}

	return SaveDefaultIfRequested(bSaveImmediately);
}

bool UMVQuestSubsystem::AddQuestObjectiveValue(
	const FName QuestId,
	const FName ObjectiveId,
	const int32 Delta,
	const bool bSaveImmediately)
{
	UMVWorldStateSubsystem* WorldState = GetWorldState();
	if (!WorldState)
	{
		return false;
	}

	const int32 CurrentValue = WorldState->GetQuestObjectiveValue(QuestId, ObjectiveId, 0);
	if (!WorldState->SetQuestObjectiveValue(QuestId, ObjectiveId, CurrentValue + Delta))
	{
		return false;
	}

	return SaveDefaultIfRequested(bSaveImmediately);
}

int32 UMVQuestSubsystem::GetQuestObjectiveValue(
	const FName QuestId,
	const FName ObjectiveId,
	const int32 DefaultValue) const
{
	const UMVWorldStateSubsystem* WorldState = GetWorldState();
	return WorldState ? WorldState->GetQuestObjectiveValue(QuestId, ObjectiveId, DefaultValue) : DefaultValue;
}

bool UMVQuestSubsystem::CompleteQuest(const FName QuestId, const bool bSaveImmediately)
{
	UMVWorldStateSubsystem* WorldState = GetWorldState();
	if (!WorldState || !WorldState->CompleteQuest(QuestId))
	{
		return false;
	}

	return SaveDefaultIfRequested(bSaveImmediately);
}

bool UMVQuestSubsystem::IsQuestCompleted(const FName QuestId) const
{
	const UMVWorldStateSubsystem* WorldState = GetWorldState();
	return WorldState && WorldState->IsQuestCompleted(QuestId);
}

UMVWorldStateSubsystem* UMVQuestSubsystem::GetWorldState() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMVWorldStateSubsystem>() : nullptr;
}

bool UMVQuestSubsystem::SaveDefaultIfRequested(const bool bSaveImmediately) const
{
	if (!bSaveImmediately)
	{
		return true;
	}

	UMVWorldStateSubsystem* WorldState = GetWorldState();
	return WorldState && WorldState->SaveToDefaultSlot();
}
