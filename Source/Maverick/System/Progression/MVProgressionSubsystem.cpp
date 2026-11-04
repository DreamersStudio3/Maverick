#include "System/Progression/MVProgressionSubsystem.h"

#include "Engine/GameInstance.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/Progression/MVProgressionCalculatorLibrary.h"
#include "System/Progression/MVProgressionDefinition.h"
#include "System/Progression/MVProgressionSettings.h"

void UMVProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UMVWorldStateSubsystem::StaticClass());

	Super::Initialize(Collection);

	ReloadDefinition();
}

void UMVProgressionSubsystem::Deinitialize()
{
	LoadedDefinition = nullptr;

	Super::Deinitialize();
}

UMVProgressionSubsystem* UMVProgressionSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;

	return GameInstance ? GameInstance->GetSubsystem<UMVProgressionSubsystem>() : nullptr;
}

bool UMVProgressionSubsystem::ReloadDefinition()
{
	const UMVProgressionSettings* Settings = GetDefault<UMVProgressionSettings>();

	if (!Settings || Settings->DefinitionAsset.IsNull())
	{
		LoadedDefinition = nullptr;
		return false;
	}

	LoadedDefinition = Settings->DefinitionAsset.LoadSynchronous();

	return IsValid(LoadedDefinition);
}

bool UMVProgressionSubsystem::IsDefinitionAvailable() const
{
	return IsValid(LoadedDefinition);
}

FMVProgressionEvaluation UMVProgressionSubsystem::EvaluateAllocation(
	const TMap<FGameplayTag, int32>& PendingRanks) const
{
	const UMVWorldStateSubsystem* WorldState = GetWorldState();

	if (!WorldState)
	{
		FMVProgressionEvaluation Evaluation;
		Evaluation.Result = EMVProgressionEvaluationResult::InvalidSaveData;
		Evaluation.Diagnostic = TEXT("World state subsystem is unavailable.");
		return Evaluation;
	}

	return UMVProgressionCalculatorLibrary::EvaluateAllocation(
		LoadedDefinition,
		WorldState->GetSaveData().PlayerProgression,
		PendingRanks);
}

FMVProgressionEvaluation UMVProgressionSubsystem::EvaluateCurrentProgression() const
{
	const TMap<FGameplayTag, int32> EmptyAllocation;
	return EvaluateAllocation(EmptyAllocation);
}

FMVProgressionEvaluation UMVProgressionSubsystem::CommitAllocation(const TMap<FGameplayTag, int32>& PendingRanks,
	int32 ExpectedRevision)
{
	UMVWorldStateSubsystem* WorldState = GetWorldState();

	if (!WorldState)
	{
		FMVProgressionEvaluation Evaluation;
		Evaluation.Result = EMVProgressionEvaluationResult::InvalidSaveData;
		Evaluation.Diagnostic = TEXT("World state subsystem is unavailable.");
		return Evaluation;
	}

	const FMVPlayerProgressionSaveData& CurrentProgression = WorldState->GetPlayerProgression();

	FMVProgressionEvaluation Evaluation =
		UMVProgressionCalculatorLibrary::EvaluateAllocation(
			LoadedDefinition,
			CurrentProgression,
			PendingRanks);

	if (CurrentProgression.Revision != ExpectedRevision)
	{
		Evaluation.Result = EMVProgressionEvaluationResult::RevisionMismatch;
		Evaluation.Diagnostic = TEXT("Progression state changed after the preview.");
		return Evaluation;
	}

	if (!Evaluation.IsSuccess())
	{
		return Evaluation;
	}

	if (Evaluation.PreviewLevel <= Evaluation.CurrentLevel)
	{
		Evaluation.Result = EMVProgressionEvaluationResult::NoAllocation;
		Evaluation.Diagnostic = TEXT("At least one pending rank is required.");
		return Evaluation;
	}

	if (CurrentProgression.Revision == MAX_int32)
	{
		Evaluation.Result = EMVProgressionEvaluationResult::StateWriteFailed;
		Evaluation.Diagnostic = TEXT("Progression revision reached its maximum value.");
		return Evaluation;
	}

	FMVPlayerProgressionSaveData NewProgression = CurrentProgression;
	NewProgression.InvestedRanks = Evaluation.PreviewRanks;
	NewProgression.Currency = Evaluation.RemainingCurrency;
	NewProgression.Revision = CurrentProgression.Revision + 1;

	if (!WorldState->TryReplacePlayerProgression(
			ExpectedRevision,
			NewProgression))
	{
		Evaluation.Result = EMVProgressionEvaluationResult::StateWriteFailed;
		Evaluation.Diagnostic = TEXT("Progression state replacement failed.");
		return Evaluation;
	}

	Evaluation.ResultRevision = NewProgression.Revision;

	Evaluation.Diagnostic.Reset();
	return Evaluation;
}

bool UMVProgressionSubsystem::AddCurrency(int64 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	UMVWorldStateSubsystem* WorldState = GetWorldState();

	if (!WorldState)
	{
		return false;
	}

	const FMVPlayerProgressionSaveData& CurrentProgression = WorldState->GetPlayerProgression();

	if (CurrentProgression.Currency > MAX_int64 - Amount
		|| CurrentProgression.Revision == MAX_int32)
	{
		return false;
	}

	FMVPlayerProgressionSaveData NewProgression = CurrentProgression;

	NewProgression.Currency += Amount;
	NewProgression.Revision = CurrentProgression.Revision + 1;

	return WorldState->TryReplacePlayerProgression(
		CurrentProgression.Revision,
		NewProgression);
}

UMVWorldStateSubsystem* UMVProgressionSubsystem::GetWorldState() const
{
	const UGameInstance* GameInstance = GetGameInstance();

	return GameInstance ? GameInstance->GetSubsystem<UMVWorldStateSubsystem>() : nullptr;
}
