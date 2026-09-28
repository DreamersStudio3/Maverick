#include "Character/PC/Progression/MVPlayerProgression.h"

#include "Character/PC/MVPlayerCharacter.h"
#include "Components/MVStatComponent.h"
#include "System/MVWorldStateSubsystem.h"
#include "System/Progression/MVProgressionSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVPlayerProgression, Log, All);

UWorld* UMVPlayerProgression::GetWorld() const
{
	if (const AMVPlayerCharacter* PlayerCharacter = OwnerPlayerCharacter.Get())
	{
		return PlayerCharacter->GetWorld();
	}

	return Super::GetWorld();
}

void UMVPlayerProgression::Initialize(AMVPlayerCharacter& InOwnerCharacter)
{
	if (OwnerPlayerCharacter.IsValid())
	{
		Deinitialize();
	}

	OwnerPlayerCharacter = &InOwnerCharacter;
	StatComponent = InOwnerCharacter.StatComponent;
	WorldStateSubsystem = UMVWorldStateSubsystem::Get(&InOwnerCharacter);
	ProgressionSubsystem = UMVProgressionSubsystem::Get(&InOwnerCharacter);

	if (UMVWorldStateSubsystem* WorldState = WorldStateSubsystem.Get())
	{
		WorldState->OnPlayerProgressionChanged.RemoveDynamic(
			this,
			&UMVPlayerProgression::HandlePlayerProgressionChanged);

		WorldState->OnPlayerProgressionChanged.AddUniqueDynamic(
			this,
			&UMVPlayerProgression::HandlePlayerProgressionChanged);
	}

	if (UMVStatComponent* Stats = StatComponent.Get())
	{
		Stats->OnBaseStatsReady.RemoveDynamic(this, &UMVPlayerProgression::HandleBaseStatsReady);

		Stats->OnBaseStatsReady.AddUniqueDynamic(this, &UMVPlayerProgression::HandleBaseStatsReady);
	}

	ApplyCurrentProgression();
}

void UMVPlayerProgression::Deinitialize()
{
	if (UMVWorldStateSubsystem* WorldState = WorldStateSubsystem.Get())
	{
		WorldState->OnPlayerProgressionChanged.RemoveDynamic(
			this,
			&UMVPlayerProgression::
				HandlePlayerProgressionChanged);
	}

	if (UMVStatComponent* Stats = StatComponent.Get())
	{
		Stats->OnBaseStatsReady.RemoveDynamic(
			this,
			&UMVPlayerProgression::HandleBaseStatsReady);
	}

	StatComponent.Reset();
	ProgressionSubsystem.Reset();
	WorldStateSubsystem.Reset();
	OwnerPlayerCharacter.Reset();
}

bool UMVPlayerProgression::ApplyCurrentProgression()
{
	UMVStatComponent* Stats = StatComponent.Get();
	UMVProgressionSubsystem* Progression = ProgressionSubsystem.Get();

	if (!Stats || !Progression)
	{
		return false;
	}

	const FMVProgressionEvaluation Evaluation = Progression->EvaluateCurrentProgression();

	if (!Evaluation.IsSuccess())
	{
		const TMap<FGameplayTag, float> EmptyBonuses;
		Stats->ReplaceProgressionStatBonuses(EmptyBonuses);

		UE_LOG(
			LogMVPlayerProgression,
			Warning,
			TEXT(
				"Failed to evaluate player progression. "
				"Owner=%s Result=%d Diagnostic=%s"),
			*GetNameSafe(OwnerPlayerCharacter.Get()),
			static_cast<int32>(Evaluation.Result),
			*Evaluation.Diagnostic);

		return false;
	}

	if (!Stats->ReplaceProgressionStatBonuses(Evaluation.CurrentStatBonuses))
	{
		UE_LOG(
			LogMVPlayerProgression,
			Warning,
			TEXT(
				"Failed to apply progression stat bonuses. "
				"Owner=%s Revision=%d"),
			*GetNameSafe(OwnerPlayerCharacter.Get()),
			Evaluation.SourceRevision);

		return false;
	}

	return true;
}

void UMVPlayerProgression::HandlePlayerProgressionChanged(const FMVPlayerProgressionSaveData& PlayerProgression)
{
	(void)PlayerProgression;
	ApplyCurrentProgression();
}

void UMVPlayerProgression::HandleBaseStatsReady(const int32 Revision)
{
	(void)Revision;
	ApplyCurrentProgression();
}