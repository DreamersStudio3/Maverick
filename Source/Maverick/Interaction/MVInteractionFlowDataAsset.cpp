#include "Interaction/MVInteractionFlowDataAsset.h"

#include "Misc/DataValidation.h"
#include "Tables/MVActionRowTableTypes.h"

const FPrimaryAssetType UMVInteractionFlowDataAsset::PrimaryAssetType(TEXT("InteractionFlow"));

FPrimaryAssetId UMVInteractionFlowDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

EDataValidationResult UMVInteractionFlowDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	auto MarkInvalid = [&Context, &Result](const FString& Message)
	{
		Context.AddError(FText::FromString(Message));
		Result = EDataValidationResult::Invalid;
	};

	if (Steps.IsEmpty())
	{
		MarkInvalid(TEXT("Interaction flow has no steps."));
		return Result;
	}

	TSet<FGameplayTag> StepIds;
	for (int32 StepIndex = 0; StepIndex < Steps.Num(); ++StepIndex)
	{
		const FInstancedStruct& StepInstance = Steps[StepIndex];
		const FMVInteractionStepData* Step = StepInstance.GetPtr<FMVInteractionStepData>();
		if (!Step)
		{
			MarkInvalid(FString::Printf(TEXT("Steps[%d] is empty or is not an interaction step."), StepIndex));
			continue;
		}

		if (!Step->StepId.IsValid())
		{
			MarkInvalid(FString::Printf(TEXT("Steps[%d] has no StepId."), StepIndex));
		}
		else if (StepIds.Contains(Step->StepId))
		{
			MarkInvalid(FString::Printf(TEXT("StepId '%s' is duplicated."), *Step->StepId.ToString()));
		}
		else
		{
			StepIds.Add(Step->StepId);
		}

		if (const FMVInteractionActionStepData* ActionStep = StepInstance.GetPtr<FMVInteractionActionStepData>())
		{
			if (!ActionStep->ActionRow.DataTable || ActionStep->ActionRow.RowName.IsNone())
			{
				MarkInvalid(FString::Printf(
					TEXT("Action step '%s' has no ActionRow."),
					*Step->StepId.ToString()));
				continue;
			}

			const UScriptStruct* RowStruct = ActionStep->ActionRow.DataTable->GetRowStruct();
			if (!RowStruct || !RowStruct->IsChildOf(FMVActionRow::StaticStruct()))
			{
				MarkInvalid(FString::Printf(
					TEXT("Action step '%s' uses invalid action table '%s'. Expected MVActionRow or child row struct."),
					*Step->StepId.ToString(),
					*GetNameSafe(ActionStep->ActionRow.DataTable)));
				continue;
			}

			const FMVActionRow* ActionRow = ActionStep->ActionRow.DataTable->FindRow<FMVActionRow>(
				ActionStep->ActionRow.RowName,
				TEXT("InteractionFlowDataAssetValidation"),
				false);
			if (!ActionRow || !ActionRow->bEnabled)
			{
				MarkInvalid(FString::Printf(
					TEXT("Action step '%s' uses missing or disabled action row '%s' in table '%s'."),
					*Step->StepId.ToString(),
					*ActionStep->ActionRow.RowName.ToString(),
					*GetNameSafe(ActionStep->ActionRow.DataTable)));
			}
		}
	}

	auto ValidateTargetStepId = [&MarkInvalid, &StepIds](const FGameplayTag TargetStepId, const FString& SourceDescription)
	{
		if (TargetStepId.IsValid() && !StepIds.Contains(TargetStepId))
		{
			MarkInvalid(FString::Printf(
				TEXT("%s targets missing step '%s'."),
				*SourceDescription,
				*TargetStepId.ToString()));
		}
	};

	if (StartStepId.IsValid())
	{
		ValidateTargetStepId(StartStepId, TEXT("StartStepId"));
	}
	else
	{
		Context.AddWarning(FText::FromString(TEXT("StartStepId is empty. The first valid step will be used as the start step.")));
	}

	for (int32 StepIndex = 0; StepIndex < Steps.Num(); ++StepIndex)
	{
		const FInstancedStruct& StepInstance = Steps[StepIndex];
		const FMVInteractionStepData* Step = StepInstance.GetPtr<FMVInteractionStepData>();
		if (!Step)
		{
			continue;
		}

		const FString StepDescription = Step->StepId.IsValid()
			? FString::Printf(TEXT("Step '%s'"), *Step->StepId.ToString())
			: FString::Printf(TEXT("Steps[%d]"), StepIndex);
		ValidateTargetStepId(Step->NextStepId, FString::Printf(TEXT("%s NextStepId"), *StepDescription));

		const FMVInteractionSelectionStepData* SelectionStep = StepInstance.GetPtr<FMVInteractionSelectionStepData>();
		if (!SelectionStep)
		{
			continue;
		}

		for (int32 TransitionIndex = 0; TransitionIndex < SelectionStep->Transitions.Num(); ++TransitionIndex)
		{
			const FMVInteractionStepTransition& Transition = SelectionStep->Transitions[TransitionIndex];
			if (Transition.TriggerName.IsNone())
			{
				MarkInvalid(FString::Printf(
					TEXT("%s transition[%d] has no TriggerName."),
					*StepDescription,
					TransitionIndex));
			}

			ValidateTargetStepId(
				Transition.NextStepId,
				FString::Printf(TEXT("%s transition[%d]"), *StepDescription, TransitionIndex));
		}
	}

	return Result == EDataValidationResult::NotValidated ? EDataValidationResult::Valid : Result;
}
