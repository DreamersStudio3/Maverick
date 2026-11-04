#include "System/Progression/MVProgressionCalculatorLibrary.h"

#include "System/Progression/MVProgressionDefinition.h"

namespace
{
	FMVProgressionEvaluation MVProgressionFailEvaluation(
		FMVProgressionEvaluation Evaluation,
		const EMVProgressionEvaluationResult Result,
		const FString& Diagnostic)
	{
		Evaluation.Result = Result;
		Evaluation.Diagnostic = Diagnostic;
		return Evaluation;
	}

	bool MVProgressionTryAddInt64(
		const int64 Left,
		const int64 Right,
		int64& OutValue)
	{
		if (Right > 0 && Left > MAX_int64 - Right)
		{
			return false;
		}

		if (Right < 0 && Left < MIN_int64 - Right)
		{
			return false;
		}

		OutValue = Left + Right;
		return true;
	}

	bool MVProgressionBuildStatBonuses(
		const UMVProgressionDefinition& Definition,
		const TMap<FGameplayTag, int32>& Ranks,
		TMap<FGameplayTag, float>& OutBonuses,
		FString& OutDiagnostic)
	{
		OutBonuses.Reset();

		for (const FMVProgressionEntryDefinition& Entry : Definition.Entries)
		{
			const int32 Rank = Ranks.FindRef(Entry.ProgressionId);

			for (const FMVProgressionStatEffect& Effect : Entry.Effects)
			{
				const FRichCurve* Curve = Effect.BonusByRank.GetRichCurveConst();

				if (!Curve || Curve->GetNumKeys() == 0)
				{
					OutDiagnostic = FString::Printf(
						TEXT("Progression '%s' has no curve keys for stat '%s'."),
						*Entry.ProgressionId.ToString(),
						*Effect.StatId.ToString());

					return false;
				}

				const float Bonus = Curve->Eval(static_cast<float>(Rank)) - Curve->Eval(0.0f);

				if (!FMath::IsFinite(Bonus))
				{
					OutDiagnostic = FString::Printf(
						TEXT("Progression '%s' produced a non-finite bonus."),
						*Entry.ProgressionId.ToString());

					return false;
				}

				OutBonuses.FindOrAdd(Effect.StatId) += Bonus;
			}
		}

		return true;
	}
}

FMVProgressionEvaluation UMVProgressionCalculatorLibrary::EvaluateAllocation(
	const UMVProgressionDefinition* Definition,
	const FMVPlayerProgressionSaveData& SaveData,
	const TMap<FGameplayTag, int32>& PendingRanks)
{
	FMVProgressionEvaluation Evaluation;
	Evaluation.SourceRevision = SaveData.Revision;
	Evaluation.ResultRevision = SaveData.Revision;
	Evaluation.CurrentCurrency = SaveData.Currency;
	Evaluation.RemainingCurrency = SaveData.Currency;

	if (!Definition)
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::MissingDefinition,
			TEXT("Progression definition is null."));
	}

	if (Definition->BaseLevel < 1
		|| Definition->MaxLevel < Definition->BaseLevel
		|| Definition->BaseLevelCost < 0
		|| Definition->CostPerLevel < 0
		|| Definition->Entries.IsEmpty())
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::InvalidDefinition,
			TEXT("Progression level or cost settings are invalid."));
	}

	if (SaveData.DataVersion <= 0
		|| SaveData.BaseLevel != Definition->BaseLevel
		|| SaveData.Currency < 0
		|| SaveData.Revision < 0)
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::InvalidSaveData,
			TEXT("Progression save data has an invalid version, base level, or currency."));
	}

	TSet<FGameplayTag> KnownProgressions;

	for (const FMVProgressionEntryDefinition& Entry : Definition->Entries)
	{
		if (!Entry.ProgressionId.IsValid()
			|| Entry.MaxRank < 0
			|| Entry.Effects.IsEmpty()
			|| KnownProgressions.Contains(Entry.ProgressionId))
		{
			return MVProgressionFailEvaluation(
				MoveTemp(Evaluation),
				EMVProgressionEvaluationResult::InvalidDefinition,
				TEXT("Progression entries require unique valid ids, non-negative limits, and effects."));
		}

		KnownProgressions.Add(Entry.ProgressionId);

		TSet<FGameplayTag> EffectStats;

		for (const FMVProgressionStatEffect& Effect : Entry.Effects)
		{
			if (!Effect.StatId.IsValid() || EffectStats.Contains(Effect.StatId))
			{
				return MVProgressionFailEvaluation(
					MoveTemp(Evaluation),
					EMVProgressionEvaluationResult::InvalidDefinition,
					TEXT("Each progression effect requires a unique valid stat id."));
			}

			EffectStats.Add(Effect.StatId);
		}
	}

	int64 CurrentInvestedRanks = 0;
	Evaluation.PreviewRanks = SaveData.InvestedRanks;

	for (const TPair<FGameplayTag, int32>& SavedRank : SaveData.InvestedRanks)
	{
		const FMVProgressionEntryDefinition* Entry = Definition->FindEntry(SavedRank.Key);

		if (!Entry)
		{
			return MVProgressionFailEvaluation(
				MoveTemp(Evaluation),
				EMVProgressionEvaluationResult::UnknownProgression,
				FString::Printf(
					TEXT("Saved progression '%s' is not registered."),
					*SavedRank.Key.ToString()));
		}

		if (SavedRank.Value < 0 || SavedRank.Value > Entry->MaxRank)
		{
			return MVProgressionFailEvaluation(
				MoveTemp(Evaluation),
				EMVProgressionEvaluationResult::InvalidSaveData,
				FString::Printf(
					TEXT("Saved rank for '%s' is outside its limit."),
					*SavedRank.Key.ToString()));
		}

		CurrentInvestedRanks += SavedRank.Value;
	}

	if (CurrentInvestedRanks > MAX_int32 - SaveData.BaseLevel)
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::InvalidSaveData,
			TEXT("Saved progression level exceeds int32 range."));
	}

	Evaluation.CurrentLevel = SaveData.BaseLevel + static_cast<int32>(CurrentInvestedRanks);

	Evaluation.PreviewLevel = Evaluation.CurrentLevel;

	if (Evaluation.CurrentLevel > Definition->MaxLevel)
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::InvalidSaveData,
			TEXT("Saved progression level exceeds the definition limit."));
	}

	int64 PendingRankTotal = 0;

	for (const TPair<FGameplayTag, int32>& PendingRank : PendingRanks)
	{
		const FMVProgressionEntryDefinition* Entry = Definition->FindEntry(PendingRank.Key);

		if (!Entry)
		{
			return MVProgressionFailEvaluation(
				MoveTemp(Evaluation),
				EMVProgressionEvaluationResult::UnknownProgression,
				FString::Printf(
					TEXT("Pending progression '%s' is not registered."),
					*PendingRank.Key.ToString()));
		}

		if (PendingRank.Value < 0)
		{
			return MVProgressionFailEvaluation(
				MoveTemp(Evaluation),
				EMVProgressionEvaluationResult::NegativeAllocation,
				FString::Printf(
					TEXT("Pending rank for '%s' is negative."),
					*PendingRank.Key.ToString()));
		}

		const int32 CurrentRank = SaveData.InvestedRanks.FindRef(PendingRank.Key);

		if (PendingRank.Value > Entry->MaxRank - CurrentRank)
		{
			return MVProgressionFailEvaluation(
				MoveTemp(Evaluation),
				EMVProgressionEvaluationResult::RankLimitExceeded,
				FString::Printf(
					TEXT("Progression '%s' exceeds its rank limit."),
					*PendingRank.Key.ToString()));
		}

		Evaluation.PreviewRanks.FindOrAdd(PendingRank.Key) = CurrentRank + PendingRank.Value;

		PendingRankTotal += PendingRank.Value;
	}

	if (PendingRankTotal > Definition->MaxLevel - Evaluation.CurrentLevel)
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::LevelLimitExceeded,
			TEXT("Pending allocation exceeds the level limit."));
	}

	Evaluation.PreviewLevel += static_cast<int32>(PendingRankTotal);

	if (!CalculateTotalCost(
		*Definition,
		Evaluation.CurrentLevel,
		static_cast<int32>(PendingRankTotal),
		Evaluation.TotalCost))
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::CostOverflow,
			TEXT("Level-up cost exceeds int64 range."));
	}

	FString CurveDiagnostic;

	if (!MVProgressionBuildStatBonuses(
			*Definition,
			SaveData.InvestedRanks,
			Evaluation.CurrentStatBonuses,
			CurveDiagnostic)
		|| !MVProgressionBuildStatBonuses(
			*Definition,
			Evaluation.PreviewRanks,
			Evaluation.PreviewStatBonuses,
			CurveDiagnostic))
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::InvalidCurve,
			CurveDiagnostic);
	}

	if (Evaluation.TotalCost > SaveData.Currency)
	{
		return MVProgressionFailEvaluation(
			MoveTemp(Evaluation),
			EMVProgressionEvaluationResult::InsufficientCurrency,
			TEXT("Currency is lower than the pending level-up cost."));
	}

	Evaluation.RemainingCurrency = SaveData.Currency - Evaluation.TotalCost;

	Evaluation.Result = EMVProgressionEvaluationResult::Success;

	Evaluation.Diagnostic.Reset();

	return Evaluation;
}

bool UMVProgressionCalculatorLibrary::CalculateTotalCost(
	const UMVProgressionDefinition& Definition,
	const int32 StartingLevel,
	const int32 LevelCount,
	int64& OutTotalCost)
{
	OutTotalCost = 0;

	if (StartingLevel < Definition.BaseLevel
		|| LevelCount < 0
		|| StartingLevel > Definition.MaxLevel
		|| LevelCount > Definition.MaxLevel - StartingLevel
		|| Definition.BaseLevelCost < 0
		|| Definition.CostPerLevel < 0)
	{
		return false;
	}

	for (int32 Index = 0; Index < LevelCount; ++Index)
	{
		const int64 LevelOffset =
			static_cast<int64>(
				StartingLevel - Definition.BaseLevel)
			+ Index;

		if (Definition.CostPerLevel > 0
			&& LevelOffset > (MAX_int64 - Definition.BaseLevelCost) / Definition.CostPerLevel)
		{
			return false;
		}

		const int64 LevelCost = Definition.BaseLevelCost + Definition.CostPerLevel * LevelOffset;

		int64 NewTotalCost = 0;

		if (!MVProgressionTryAddInt64(
			OutTotalCost,
			LevelCost,
			NewTotalCost))
		{
			return false;
		}

		OutTotalCost = NewTotalCost;
	}

	return true;
}