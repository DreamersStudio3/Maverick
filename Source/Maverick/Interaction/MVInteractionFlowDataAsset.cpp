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

		const FMVInteractionChoiceStepData* ChoiceStep = StepInstance.GetPtr<FMVInteractionChoiceStepData>();
		if (ChoiceStep)
		{
			if (ChoiceStep->ChoiceData.Choices.IsEmpty())
			{
				MarkInvalid(FString::Printf(
					TEXT("%s has no choices."),
					*StepDescription));
			}

			TSet<FGameplayTag> ChoiceIds;
			for (int32 ChoiceIndex = 0; ChoiceIndex < ChoiceStep->ChoiceData.Choices.Num(); ++ChoiceIndex)
			{
				const FMVInteractionChoiceEntryData& Choice = ChoiceStep->ChoiceData.Choices[ChoiceIndex];
				if (!Choice.ChoiceId.IsValid())
				{
					MarkInvalid(FString::Printf(
						TEXT("%s choice[%d] has no ChoiceId."),
						*StepDescription,
						ChoiceIndex));
				}
				else if (ChoiceIds.Contains(Choice.ChoiceId))
				{
					MarkInvalid(FString::Printf(
						TEXT("%s choice id '%s' is duplicated."),
						*StepDescription,
						*Choice.ChoiceId.ToString()));
				}
				else
				{
					ChoiceIds.Add(Choice.ChoiceId);
				}

				ValidateTargetStepId(
					Choice.NextStepId,
					FString::Printf(TEXT("%s choice[%d]"), *StepDescription, ChoiceIndex));
			}
		}

		const FMVInteractionSelectionStepData* SelectionStep = StepInstance.GetPtr<FMVInteractionSelectionStepData>();
		if (!SelectionStep)
		{
			continue;
		}

		TSet<FGameplayTag> EntryIds;
		TSet<FGameplayTag> MenuIds;
		if (SelectionStep->MenuData.RootMenuId.IsValid())
		{
			MenuIds.Add(SelectionStep->MenuData.RootMenuId);
		}
		for (int32 EntryIndex = 0; EntryIndex < SelectionStep->MenuData.Entries.Num(); ++EntryIndex)
		{
			const FMVMenuEntryData& Entry = SelectionStep->MenuData.Entries[EntryIndex];
			if (!Entry.EntryId.IsValid())
			{
				MarkInvalid(FString::Printf(
					TEXT("%s menu entry[%d] has no EntryId."),
					*StepDescription,
					EntryIndex));
			}
			else if (EntryIds.Contains(Entry.EntryId))
			{
				MarkInvalid(FString::Printf(
					TEXT("%s menu entry id '%s' is duplicated."),
					*StepDescription,
					*Entry.EntryId.ToString()));
			}
			else
			{
				EntryIds.Add(Entry.EntryId);
			}

			if (Entry.SubMenuId.IsValid())
			{
				MenuIds.Add(Entry.SubMenuId);
			}
		}
		for (int32 EntryIndex = 0; EntryIndex < SelectionStep->MenuData.Entries.Num(); ++EntryIndex)
		{
			const FMVMenuEntryData& Entry = SelectionStep->MenuData.Entries[EntryIndex];
			if (Entry.ParentMenuId.IsValid() && !MenuIds.Contains(Entry.ParentMenuId))
			{
				MarkInvalid(FString::Printf(
					TEXT("%s menu entry '%s' points to missing ParentMenuId '%s'."),
					*StepDescription,
					*Entry.EntryId.ToString(),
					*Entry.ParentMenuId.ToString()));
			}
		}

		for (int32 TransitionIndex = 0; TransitionIndex < SelectionStep->Transitions.Num(); ++TransitionIndex)
		{
			const FMVInteractionStepTransition& Transition = SelectionStep->Transitions[TransitionIndex];
			if (!Transition.TriggerId.IsValid())
			{
				MarkInvalid(FString::Printf(
					TEXT("%s transition[%d] has no TriggerId."),
					*StepDescription,
					TransitionIndex));
			}
			else if (!EntryIds.Contains(Transition.TriggerId))
			{
				MarkInvalid(FString::Printf(
					TEXT("%s transition[%d] trigger '%s' does not match any menu EntryId."),
					*StepDescription,
					TransitionIndex,
					*Transition.TriggerId.ToString()));
			}

			ValidateTargetStepId(
				Transition.NextStepId,
				FString::Printf(TEXT("%s transition[%d]"), *StepDescription, TransitionIndex));
		}
	}

	return Result == EDataValidationResult::NotValidated ? EDataValidationResult::Valid : Result;
}
