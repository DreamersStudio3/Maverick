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

	auto HasAnyActionRowValue = [](const FDataTableRowHandle& ActionRow)
	{
		return ActionRow.DataTable || !ActionRow.RowName.IsNone();
	};

	auto ValidateActionRowHandle = [&HasAnyActionRowValue, &MarkInvalid](
		const FDataTableRowHandle& ActionRow,
		const FString& SourceDescription,
		const bool bRequired)
	{
		if (!HasAnyActionRowValue(ActionRow))
		{
			if (bRequired)
			{
				MarkInvalid(FString::Printf(TEXT("%s has no ActionRow."), *SourceDescription));
			}
			return;
		}

		if (!ActionRow.DataTable || ActionRow.RowName.IsNone())
		{
			MarkInvalid(FString::Printf(
				TEXT("%s has incomplete ActionRow."),
				*SourceDescription));
			return;
		}

		const UScriptStruct* RowStruct = ActionRow.DataTable->GetRowStruct();
		if (!RowStruct || !RowStruct->IsChildOf(FMVActionRow::StaticStruct()))
		{
			MarkInvalid(FString::Printf(
				TEXT("%s uses invalid action table '%s'. Expected MVActionRow or child row struct."),
				*SourceDescription,
				*GetNameSafe(ActionRow.DataTable)));
			return;
		}

		const FMVActionRow* FoundActionRow = ActionRow.DataTable->FindRow<FMVActionRow>(
			ActionRow.RowName,
			TEXT("InteractionFlowDataAssetValidation"),
			false);
		if (!FoundActionRow || !FoundActionRow->bEnabled)
		{
			MarkInvalid(FString::Printf(
				TEXT("%s uses missing or disabled action row '%s' in table '%s'."),
				*SourceDescription,
				*ActionRow.RowName.ToString(),
				*GetNameSafe(ActionRow.DataTable)));
		}
	};

	auto ValidateCommandInstance = [&MarkInvalid, &ValidateActionRowHandle](
		const FInstancedStruct& CommandInstance,
		const FString& SourceDescription)
	{
		const FMVInteractionCommandData* Command = CommandInstance.GetPtr<FMVInteractionCommandData>();
		if (!Command)
		{
			MarkInvalid(FString::Printf(
				TEXT("%s is empty or is not an interaction command."),
				*SourceDescription));
			return;
		}

		if (const FMVInteractionPlayActionCommandData* PlayActionCommand =
			CommandInstance.GetPtr<FMVInteractionPlayActionCommandData>())
		{
			ValidateActionRowHandle(
				PlayActionCommand->ActionRow,
				FString::Printf(TEXT("%s PlayAction"), *SourceDescription),
				true);
			return;
		}

		if (const FMVInteractionGameplayEventCommandData* EventCommand =
			CommandInstance.GetPtr<FMVInteractionGameplayEventCommandData>())
		{
			if (!EventCommand->EventTag.IsValid())
			{
				MarkInvalid(FString::Printf(
					TEXT("%s GameplayEvent has no EventTag."),
					*SourceDescription));
			}
			return;
		}

		MarkInvalid(FString::Printf(
			TEXT("%s uses unsupported command type."),
			*SourceDescription));
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
			if (ActionStep->Commands.IsEmpty())
			{
				MarkInvalid(FString::Printf(
					TEXT("Action step '%s' has no commands."),
					*Step->StepId.ToString()));
				continue;
			}

			for (int32 CommandIndex = 0; CommandIndex < ActionStep->Commands.Num(); ++CommandIndex)
			{
				ValidateCommandInstance(
					ActionStep->Commands[CommandIndex],
					FString::Printf(
						TEXT("Action step '%s' command[%d]"),
						*Step->StepId.ToString(),
						CommandIndex));
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
		auto ValidateMenuEntry = [&EntryIds, &MarkInvalid, &ValidateCommandInstance](
			const FMVMenuEntryData& Entry,
			const FString& EntryDescription)
		{
			if (!Entry.EntryId.IsValid())
			{
				MarkInvalid(FString::Printf(
					TEXT("%s has no EntryId."),
					*EntryDescription));
			}
			else if (EntryIds.Contains(Entry.EntryId))
			{
				MarkInvalid(FString::Printf(
					TEXT("%s id '%s' is duplicated."),
					*EntryDescription,
					*Entry.EntryId.ToString()));
			}
			else
			{
				EntryIds.Add(Entry.EntryId);
			}

			for (int32 CommandIndex = 0; CommandIndex < Entry.Commands.Num(); ++CommandIndex)
			{
				ValidateCommandInstance(
					Entry.Commands[CommandIndex],
					FString::Printf(TEXT("%s command[%d]"), *EntryDescription, CommandIndex));
			}
		};

		auto ValidateMenuEntries = [
			&Context,
			&StepDescription,
			&ValidateMenuEntry
		](auto&& ValidateMenuEntriesRef, const TArray<FMVMenuEntryData>& Entries, const FString& EntriesDescription) -> void
		{
			for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
			{
				const FMVMenuEntryData& Entry = Entries[EntryIndex];
				const FString EntryDescription = FString::Printf(
					TEXT("%s %s entry[%d]"),
					*StepDescription,
					*EntriesDescription,
					EntryIndex);
				ValidateMenuEntry(Entry, EntryDescription);

				if (!Entry.SubMenu)
				{
					continue;
				}

				if (Entry.SubMenu->Entries.IsEmpty())
				{
					Context.AddWarning(FText::FromString(FString::Printf(
						TEXT("%s has SubMenu but no entries."),
						*EntryDescription)));
					continue;
				}

				ValidateMenuEntriesRef(
					ValidateMenuEntriesRef,
					Entry.SubMenu->Entries,
					FString::Printf(TEXT("%s entry[%d] submenu"), *EntriesDescription, EntryIndex));
			}
		};

		if (SelectionStep->MenuData.Entries.IsEmpty())
		{
			MarkInvalid(FString::Printf(
				TEXT("%s has no menu entries."),
				*StepDescription));
		}
		else
		{
			ValidateMenuEntries(ValidateMenuEntries, SelectionStep->MenuData.Entries, TEXT("root menu"));
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
