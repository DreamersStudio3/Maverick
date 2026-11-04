#include "Components/MVStatComponent.h"
#include "GameFramework/Character.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/Progression/MVProgressionSubsystem.h"
#include "Tags/MVGameplayTags.h"
#include "Engine/World.h"

#if !UE_BUILD_SHIPPING

DEFINE_LOG_CATEGORY_STATIC(
	LogMVProgressionDebug,
	Log,
	All);

namespace
{
	const FString MVProgressionDebugTestSaveSlotName = TEXT("Maverick_LevelUpTest");
	
	UMVProgressionSubsystem*
	MVProgressionDebugResolveSubsystem(UWorld* World)
	{
		return World
			? UMVProgressionSubsystem::Get(World)
			: nullptr;
	}

	bool MVProgressionDebugResolveProgressionId(
		const FString& Name,
		FGameplayTag& OutProgressionId)
	{
		if (Name.Equals(
			TEXT("HP"),
			ESearchCase::IgnoreCase))
		{
			OutProgressionId =
				MVGameplayTags::Progression_Attribute_HP;
			return true;
		}

		if (Name.Equals(
			TEXT("Stamina"),
			ESearchCase::IgnoreCase))
		{
			OutProgressionId =
				MVGameplayTags::
					Progression_Attribute_Stamina;
			return true;
		}

		if (Name.Equals(
			TEXT("MP"),
			ESearchCase::IgnoreCase))
		{
			OutProgressionId =
				MVGameplayTags::Progression_Attribute_MP;
			return true;
		}

		return false;
	}

	void MVProgressionDebugAddCurrencyCommand(
		const TArray<FString>& Args,
		UWorld* World)
	{
		int64 Amount = 0;

		if (Args.Num() != 1
			|| !LexTryParseString(
				Amount,
				*Args[0])
			|| Amount <= 0)
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"Usage: "
					"MV.Progression.Currency.Add "
					"<PositiveAmount>"));
			return;
		}

		UMVProgressionSubsystem* Progression =
			MVProgressionDebugResolveSubsystem(World);

		if (!Progression
			|| !Progression->AddCurrency(Amount))
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"ProgressionDebug currency "
					"addition failed. Amount=%lld"),
				static_cast<long long>(Amount));
			return;
		}

		const FMVProgressionEvaluation Evaluation =
			Progression->EvaluateCurrentProgression();

		UE_LOG(
			LogMVProgressionDebug,
			Display,
			TEXT(
				"ProgressionDebug currency added. "
				"Amount=%lld Currency=%lld "
				"Revision=%d"),
			static_cast<long long>(Amount),
			static_cast<long long>(
				Evaluation.CurrentCurrency),
			Evaluation.SourceRevision);
	}

	void MVProgressionDebugLevelUpCommand(
		const TArray<FString>& Args,
		UWorld* World)
	{
		int32 RankCount = 1;
		FGameplayTag ProgressionId;

		if (Args.IsEmpty()
			|| Args.Num() > 2
			|| !MVProgressionDebugResolveProgressionId(
				Args[0],
				ProgressionId)
			|| (Args.Num() == 2
				&& (!LexTryParseString(
						RankCount,
						*Args[1])
					|| RankCount <= 0)))
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"Usage: MV.Progression.LevelUp "
					"<HP|Stamina|MP> "
					"[PositiveRankCount]"));
			return;
		}

		UMVProgressionSubsystem* Progression =
			MVProgressionDebugResolveSubsystem(World);

		if (!Progression)
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"ProgressionDebug subsystem "
					"is unavailable."));
			return;
		}

		TMap<FGameplayTag, int32> PendingRanks;
		PendingRanks.Add(
			ProgressionId,
			RankCount);

		const FMVProgressionEvaluation Preview =
			Progression->EvaluateAllocation(
				PendingRanks);

		if (!Preview.IsSuccess())
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"ProgressionDebug preview failed. "
					"Result=%d Diagnostic=%s"),
				static_cast<int32>(Preview.Result),
				*Preview.Diagnostic);
			return;
		}

		const FMVProgressionEvaluation Commit =
			Progression->CommitAllocation(
				PendingRanks,
				Preview.SourceRevision);

		if (!Commit.IsSuccess())
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"ProgressionDebug commit failed. "
					"Result=%d Diagnostic=%s"),
				static_cast<int32>(Commit.Result),
				*Commit.Diagnostic);
			return;
		}

		UE_LOG(
			LogMVProgressionDebug,
			Display,
			TEXT(
				"ProgressionDebug level up complete. "
				"Attribute=%s RankCount=%d "
				"Level=%d Cost=%lld Currency=%lld "
				"Revision=%d"),
			*ProgressionId.ToString(),
			RankCount,
			Commit.PreviewLevel,
			static_cast<long long>(
				Commit.TotalCost),
			static_cast<long long>(
				Commit.RemainingCurrency),
			Commit.ResultRevision);
	}

	void MVProgressionDebugStatusCommand(
		const TArray<FString>& Args,
		UWorld* World)
	{
		(void)Args;

		UMVProgressionSubsystem* Progression =
			MVProgressionDebugResolveSubsystem(World);

		UMVWorldStateSubsystem* WorldState =
			World
				? UMVWorldStateSubsystem::Get(World)
				: nullptr;

		if (!Progression || !WorldState)
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"ProgressionDebug subsystem "
					"is unavailable."));
			return;
		}

		const FMVProgressionEvaluation Evaluation =
			Progression->EvaluateCurrentProgression();

		if (!Evaluation.IsSuccess())
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT(
					"ProgressionDebug status failed. "
					"Result=%d Diagnostic=%s"),
				static_cast<int32>(Evaluation.Result),
				*Evaluation.Diagnostic);
			return;
		}

		const FMVPlayerProgressionSaveData& SaveData =
			WorldState->GetPlayerProgression();

		const ACharacter* PlayerCharacter =
			UGameplayStatics::GetPlayerCharacter(
				World,
				0);

		const UMVStatComponent* Stats =
			PlayerCharacter
				? PlayerCharacter
					->FindComponentByClass<
						UMVStatComponent>()
				: nullptr;

		UE_LOG(
			LogMVProgressionDebug,
			Display,
			TEXT(
				"ProgressionDebug status. "
				"Level=%d Currency=%lld Revision=%d "
				"HP=%d Stamina=%d MP=%d "
				"MaxHP=%.2f MaxStamina=%.2f "
				"MaxMP=%.2f"),
			Evaluation.CurrentLevel,
			static_cast<long long>(
				SaveData.Currency),
			SaveData.Revision,
			SaveData.InvestedRanks.FindRef(
				MVGameplayTags::
					Progression_Attribute_HP),
			SaveData.InvestedRanks.FindRef(
				MVGameplayTags::
					Progression_Attribute_Stamina),
			SaveData.InvestedRanks.FindRef(
				MVGameplayTags::
					Progression_Attribute_MP),
			Stats ? Stats->MaxHP : 0.0f,
			Stats ? Stats->MaxStamina : 0.0f,
			Stats ? Stats->MaxMP : 0.0f);
	}

	void MVProgressionDebugSaveTestCommand(
	const TArray<FString>& Args,
	UWorld* World)
	{
		if (!Args.IsEmpty() || !World || !World->IsGameWorld())
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT("Run MV.Progression.SaveTest during PIE with no arguments."));
			return;
		}

		UMVWorldStateSubsystem* WorldState =
			UMVWorldStateSubsystem::Get(World);

		if (!WorldState
			|| !WorldState->SaveToSlot(
				MVProgressionDebugTestSaveSlotName,
				0))
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT("ProgressionDebug test save failed. Slot=%s"),
				*MVProgressionDebugTestSaveSlotName);
			return;
		}

		UE_LOG(
			LogMVProgressionDebug,
			Display,
			TEXT("ProgressionDebug test save complete. Slot=%s UserIndex=0"),
			*MVProgressionDebugTestSaveSlotName);

		MVProgressionDebugStatusCommand(Args, World);
	}

	void MVProgressionDebugLoadTestCommand(
		const TArray<FString>& Args,
		UWorld* World)
	{
		if (!Args.IsEmpty() || !World || !World->IsGameWorld())
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT("Run MV.Progression.LoadTest during PIE with no arguments."));
			return;
		}

		UMVWorldStateSubsystem* WorldState =
			UMVWorldStateSubsystem::Get(World);

		if (!WorldState
			|| !WorldState->LoadFromSlot(
				MVProgressionDebugTestSaveSlotName,
				0))
		{
			UE_LOG(
				LogMVProgressionDebug,
				Warning,
				TEXT("ProgressionDebug test load failed. Slot=%s"),
				*MVProgressionDebugTestSaveSlotName);
			return;
		}

		UE_LOG(
			LogMVProgressionDebug,
			Display,
			TEXT("ProgressionDebug test load complete. Slot=%s UserIndex=0"),
			*MVProgressionDebugTestSaveSlotName);

		MVProgressionDebugStatusCommand(Args, World);
	}
	
	FAutoConsoleCommandWithWorldAndArgs
	MVProgressionDebugAddCurrencyConsoleCommand(
		TEXT("MV.Progression.Currency.Add"),
		TEXT(
			"Add test progression currency. "
			"Usage: MV.Progression.Currency.Add 10000"),
		FConsoleCommandWithWorldAndArgsDelegate::
			CreateStatic(
				&MVProgressionDebugAddCurrencyCommand));

	FAutoConsoleCommandWithWorldAndArgs
	MVProgressionDebugLevelUpConsoleCommand(
		TEXT("MV.Progression.LevelUp"),
		TEXT(
			"Commit a test level up. "
			"Usage: MV.Progression.LevelUp HP 1"),
		FConsoleCommandWithWorldAndArgsDelegate::
			CreateStatic(
				&MVProgressionDebugLevelUpCommand));

	FAutoConsoleCommandWithWorldAndArgs
	MVProgressionDebugStatusConsoleCommand(
		TEXT("MV.Progression.Status"),
		TEXT(
			"Print current progression and "
			"maximum resource stats."),
		FConsoleCommandWithWorldAndArgsDelegate::
			CreateStatic(
				&MVProgressionDebugStatusCommand));
	
	FAutoConsoleCommandWithWorldAndArgs
	MVProgressionDebugSaveTestConsoleCommand(
	TEXT("MV.Progression.SaveTest"),
	TEXT("Save confirmed world state to Maverick_LevelUpTest."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
		&MVProgressionDebugSaveTestCommand));

	FAutoConsoleCommandWithWorldAndArgs
	MVProgressionDebugLoadTestConsoleCommand(
		TEXT("MV.Progression.LoadTest"),
		TEXT("Load world state from Maverick_LevelUpTest."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
			&MVProgressionDebugLoadTestCommand));
}

#endif