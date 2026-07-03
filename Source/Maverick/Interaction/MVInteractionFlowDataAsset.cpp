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
			ValidateActionRowHandle(
				ActionStep->ActionRow,
				FString::Printf(TEXT("Action step '%s'"), *Step->StepId.ToString()),
				true);
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

		for (int32 SubMenuIndex = 0; SubMenuIndex < SelectionStep->MenuData.SubMenus.Num(); ++SubMenuIndex)
		{
			const FMVInteractionMenuPageData& SubMenu = SelectionStep->MenuData.SubMenus[SubMenuIndex];
			if (!SubMenu.MenuId.IsValid())
			{
				MarkInvalid(FString::Printf(
					TEXT("%s submenu[%d] has no MenuId."),
					*StepDescription,
					SubMenuIndex));
			}
			else if (MenuIds.Contains(SubMenu.MenuId))
			{
				MarkInvalid(FString::Printf(
					TEXT("%s submenu id '%s' is duplicated."),
					*StepDescription,
					*SubMenu.MenuId.ToString()));
			}
			else
			{
				MenuIds.Add(SubMenu.MenuId);
			}
		}

		auto ValidateMenuEntry = [&EntryIds, &MarkInvalid, &ValidateActionRowHandle](
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

			ValidateActionRowHandle(Entry.ActionRow, FString::Printf(TEXT("%s ActionRow"), *EntryDescription), false);
		};

		for (int32 EntryIndex = 0; EntryIndex < SelectionStep->MenuData.Entries.Num(); ++EntryIndex)
		{
			ValidateMenuEntry(
				SelectionStep->MenuData.Entries[EntryIndex],
				FString::Printf(TEXT("%s root menu entry[%d]"), *StepDescription, EntryIndex));
		}

		for (int32 SubMenuIndex = 0; SubMenuIndex < SelectionStep->MenuData.SubMenus.Num(); ++SubMenuIndex)
		{
			const FMVInteractionMenuPageData& SubMenu = SelectionStep->MenuData.SubMenus[SubMenuIndex];
			for (int32 EntryIndex = 0; EntryIndex < SubMenu.Entries.Num(); ++EntryIndex)
			{
				ValidateMenuEntry(
					SubMenu.Entries[EntryIndex],
					FString::Printf(
						TEXT("%s submenu[%d] entry[%d]"),
						*StepDescription,
						SubMenuIndex,
						EntryIndex));
			}
		}

		auto ValidateMenuEntryLinks = [&MenuIds, &MarkInvalid](
			const FMVMenuEntryData& Entry,
			const FString& EntryDescription)
		{
			if (Entry.ParentMenuId.IsValid() && !MenuIds.Contains(Entry.ParentMenuId))
			{
				MarkInvalid(FString::Printf(
					TEXT("%s points to missing ParentMenuId '%s'."),
					*EntryDescription,
					*Entry.ParentMenuId.ToString()));
			}

			if (Entry.SubMenuId.IsValid() && !MenuIds.Contains(Entry.SubMenuId))
			{
				MarkInvalid(FString::Printf(
					TEXT("%s points to missing SubMenuId '%s'. Add a SubMenus page with the same MenuId."),
					*EntryDescription,
					*Entry.SubMenuId.ToString()));
			}
		};

		for (int32 EntryIndex = 0; EntryIndex < SelectionStep->MenuData.Entries.Num(); ++EntryIndex)
		{
			const FMVMenuEntryData& Entry = SelectionStep->MenuData.Entries[EntryIndex];
			ValidateMenuEntryLinks(
				Entry,
				FString::Printf(TEXT("%s root menu entry[%d] '%s'"), *StepDescription, EntryIndex, *Entry.EntryId.ToString()));
		}

		for (int32 SubMenuIndex = 0; SubMenuIndex < SelectionStep->MenuData.SubMenus.Num(); ++SubMenuIndex)
		{
			const FMVInteractionMenuPageData& SubMenu = SelectionStep->MenuData.SubMenus[SubMenuIndex];
			for (int32 EntryIndex = 0; EntryIndex < SubMenu.Entries.Num(); ++EntryIndex)
			{
				const FMVMenuEntryData& Entry = SubMenu.Entries[EntryIndex];
				ValidateMenuEntryLinks(
					Entry,
					FString::Printf(
						TEXT("%s submenu[%d] entry[%d] '%s'"),
						*StepDescription,
						SubMenuIndex,
						EntryIndex,
						*Entry.EntryId.ToString()));
			}
		}

		if (SelectionStep->MenuData.Entries.IsEmpty() && SelectionStep->MenuData.SubMenus.IsEmpty())
		{
			MarkInvalid(FString::Printf(
				TEXT("%s has no menu entries or submenus."),
				*StepDescription));
		}

		for (int32 SubMenuIndex = 0; SubMenuIndex < SelectionStep->MenuData.SubMenus.Num(); ++SubMenuIndex)
		{
			const FMVInteractionMenuPageData& SubMenu = SelectionStep->MenuData.SubMenus[SubMenuIndex];
			if (SubMenu.Entries.IsEmpty())
			{
				Context.AddWarning(FText::FromString(FString::Printf(
					TEXT("%s submenu '%s' has no entries."),
					*StepDescription,
					*SubMenu.MenuId.ToString())));
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
