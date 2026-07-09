// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVCombatComponent.h"

#include "Chooser.h"
#include "ChooserFunctionLibrary.h"
#include "AI/MVAICombatTypes.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVActionComponent.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Engine/DataTable.h"
#include "Public/Interface/MVAbilityInterface.h"
#include "Tags/MVGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVCombatComponent, Log, All);

namespace
{
FString MVCombatActionTypeToString(const EMVCombatActionTypes ActionType)
{
	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
	return EnumPtr
		? EnumPtr->GetNameStringByValue(static_cast<int64>(ActionType))
		: FString(TEXT("None"));
}

FString MVCombatRowHandleToString(const FDataTableRowHandle& RowHandle)
{
	return FString::Printf(
		TEXT("%s.%s"),
		*GetNameSafe(RowHandle.DataTable),
		*RowHandle.RowName.ToString());
}

FString MVCombatSkillRowNamesToString(const TArray<FName>& RowNames)
{
	FString Result;
	for (int32 Index = 0; Index < RowNames.Num(); ++Index)
	{
		if (Index > 0)
		{
			Result += TEXT(" -> ");
		}

		Result += RowNames[Index].ToString();
	}

	return Result;
}

void MVCombatAppendRowCandidate(TArray<FName>& OutCandidates, const TCHAR* RowName)
{
	OutCandidates.Add(FName(RowName));
}

TArray<FName> MVCombatMakeFallbackAttackRowCandidates(const EMVCombatActionTypes ActionType)
{
	TArray<FName> Candidates;
	switch (ActionType)
	{
	case EMVCombatActionTypes::LightAttack:
		MVCombatAppendRowCandidate(Candidates, TEXT("LightAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("LightAttack1"));
		break;
	case EMVCombatActionTypes::HeavyAttack:
		MVCombatAppendRowCandidate(Candidates, TEXT("HeavyAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("HeavyAttack1"));
		break;
	case EMVCombatActionTypes::ChargeAttack:
		MVCombatAppendRowCandidate(Candidates, TEXT("ChargeAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("ChargeAttack1"));
		break;
	case EMVCombatActionTypes::SprintLightAttack:
		MVCombatAppendRowCandidate(Candidates, TEXT("Sprint_LightAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("SprintLightAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("SprintLightAttack1"));
		break;
	case EMVCombatActionTypes::SprintHeavyAttack:
		MVCombatAppendRowCandidate(Candidates, TEXT("Sprint_HeavyAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("SprintHeavyAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("SprintHeavyAttack1"));
		break;
	case EMVCombatActionTypes::DodgeLightAttack:
		MVCombatAppendRowCandidate(Candidates, TEXT("Dodge_LightAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("DodgeLightAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("DodgeLightAttack1"));
		break;
	case EMVCombatActionTypes::DodgeHeavyAttack:
		MVCombatAppendRowCandidate(Candidates, TEXT("Dodge_HeavyAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("DodgeHeavyAttack"));
		MVCombatAppendRowCandidate(Candidates, TEXT("DodgeHeavyAttack1"));
		break;
	default:
		break;
	}

	return Candidates;
}

bool MVCombatIsBasicAttackStartRowCandidate(const EMVCombatActionTypes ActionType, const FName RowName)
{
	return MVCombatMakeFallbackAttackRowCandidates(ActionType).Contains(RowName);
}

bool MVCombatIsSkillStartRowName(const FName RowName)
{
	return RowName.ToString().StartsWith(TEXT("Skill"));
}

bool MVCombatIsDodgeContextualAttackRowName(const FString& RowName)
{
	return RowName.StartsWith(TEXT("Dodge_LightAttack"))
		|| RowName.StartsWith(TEXT("Dodge_HeavyAttack"))
		|| RowName.StartsWith(TEXT("DodgeLightAttack"))
		|| RowName.StartsWith(TEXT("DodgeHeavyAttack"));
}

bool MVCombatIsSprintContextualBasicAttackActionType(const EMVCombatActionTypes ActionType)
{
	return ActionType == EMVCombatActionTypes::SprintLightAttack
		|| ActionType == EMVCombatActionTypes::SprintHeavyAttack;
}

bool MVCombatIsDodgeContextualBasicAttackActionType(const EMVCombatActionTypes ActionType)
{
	return ActionType == EMVCombatActionTypes::DodgeLightAttack
		|| ActionType == EMVCombatActionTypes::DodgeHeavyAttack;
}

FName MVCombatMakeSkillMapKey(const int32 SkillIndex)
{
	return FName(*FString::Printf(TEXT("Skill%d"), SkillIndex));
}

FName MVCombatActionTableNameFromDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

EMVAttackSwingDirection MVCombatGetOppositeSwingDirection(const EMVAttackSwingDirection Direction)
{
	switch (Direction)
	{
	case EMVAttackSwingDirection::RightToLeft:
		return EMVAttackSwingDirection::LeftToRight;
	case EMVAttackSwingDirection::LeftToRight:
		return EMVAttackSwingDirection::RightToLeft;
	default:
		return EMVAttackSwingDirection::None;
	}
}

}

// Sets default values for this component's properties
UMVCombatComponent::UMVCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMVCombatComponent::BeginPlay()
{
	RefreshActionMaps();




	// This goes to the UActorComponent BeginPlay()
	// And UActorComponent will goes to the Blueprint Function If Blueprint exist
	Super::BeginPlay();


	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
	{
		InputManager->RegisterActionInputHandler(this, MVActionInputHandlerPriorities::Combat);
	}

	if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
	{
		ActionComponent->OnActionEnded.RemoveDynamic(this, &UMVCombatComponent::HandleActionEnded);
		ActionComponent->OnActionEnded.AddUniqueDynamic(this, &UMVCombatComponent::HandleActionEnded);
	}

	StatComponent = OwnerCharacter->FindComponentByClass<UMVStatComponent>();
	if (!StatComponent)
	{
		return;
	}
	
}

void UMVCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner()))
	{
		if (UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent)
		{
			InputManager->UnregisterActionInputHandler(this);
		}

		if (UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent)
		{
			ActionComponent->OnActionEnded.RemoveDynamic(this, &UMVCombatComponent::HandleActionEnded);
		}
	}

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UMVCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateContextualBasicAttackResets();
	TryUpdateHeavyChargeAttack(0.0f);
}

bool UMVCombatComponent::TryCombatAction(
	EMVCombatActionTypes InActionType,
	int32 ActionIndex,
	FName StartSection)
{
	const int32 ResolvedActionIndex = FMath::Max(0, ActionIndex);

	// Check Input State
	AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter)
	{
		return false;
	}

	UMVActionComponent* ActionComponent = OwnerCharacter->ActionComponent;
	if (!ActionComponent)
	{
		return false;
	}
	
	bool bIsRevoveryEscapeWindowOpen = false;
	UMVInputManagerComponent* InputManager = OwnerCharacter->InputManagerComponent;
	if (!InputManager)
	{
		// In case of not Player
		bIsRevoveryEscapeWindowOpen = true;
	}
	else
	{
		// In case of Player, Check Recovery Escape Window
		bIsRevoveryEscapeWindowOpen =InputManager->IsRecoveryEscapeWindowOpen();
	}
	
	const bool bActionRunning = ActionComponent->IsActionRunning();
	const bool bCanStartAction = !bActionRunning
		|| (bIsRevoveryEscapeWindowOpen && ActionComponent->CanInterruptActiveAction());

	if (bCanStartAction)
	{
		const EMVCombatActionTypes ResolvedActionType = ResolveContextualBasicAttackActionType(InActionType);
		UE_LOG(
			LogMVCombatComponent,
			Log,
			TEXT("TryCombatAction: Requested=%s Resolved=%s"),
			*MVCombatActionTypeToString(InActionType),
			*MVCombatActionTypeToString(ResolvedActionType));

		if (IsBasicAttackActionType(ResolvedActionType))
		{
			const bool bStarted = TryBasicAttack(ResolvedActionType, ResolvedActionIndex, StartSection);
			if (bStarted)
			{
				MarkContextualBasicAttackStarted(ResolvedActionType);
			}
			return bStarted;
		}
		else if (ResolvedActionType == EMVCombatActionTypes::Skill)
		{
			return TrySkill(ResolvedActionType, ResolvedActionIndex, StartSection);
		}

		// Other Actions --> If Other action should concern, add logic
	}
	
	return false;
}

bool UMVCombatComponent::TryHandleActionInput(
	const FGameplayTag ActionInputTag,
	const FVector2D /*ControllerSpaceInput*/,
	const bool /*bHasMovementInput*/)
{
	if (!IsCombatActionInputTag(ActionInputTag))
	{
		return false;
	}

	return ChooseTryCombatAction(ActionInputTag);
}

bool UMVCombatComponent::TryHandleHoldActionInput(
	const FGameplayTag ActionInputTag,
	const EMVActionInputPhase Phase,
	const float HeldSeconds,
	const FVector2D /*ControllerSpaceInput*/,
	const bool /*bHasMovementInput*/)
{
	if (!IsHeavyChargeActionInputTag(ActionInputTag))
	{
		return false;
	}

	switch (Phase)
	{
	case EMVActionInputPhase::Started:
		return TryBeginHeavyChargeAttack(HeldSeconds);
	case EMVActionInputPhase::Triggered:
		return TryUpdateHeavyChargeAttack(HeldSeconds);
	case EMVActionInputPhase::Completed:
		return TryReleaseHeavyChargeAttack(HeldSeconds, false);
	case EMVActionInputPhase::Canceled:
		return TryReleaseHeavyChargeAttack(HeldSeconds, true);
	default:
		return false;
	}
}

bool UMVCombatComponent::ChooseTryCombatAction(const FGameplayTag ActionInputTag)
{
	UE_LOG(
		LogMVCombatComponent,
		Log,
		TEXT("ChooseTryCombatAction: InputTag=%s"),
		*ActionInputTag.ToString());

	UpdateContextualBasicAttackResets();

	if (ActionInputTag.MatchesTag(MVGameplayTags::Action_Input_Skill))
	{
		const int32 SkillIndex = ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_Skill_R)
			? 1
			: 0;
		return TryCombatAction(EMVCombatActionTypes::Skill, SkillIndex);
	}
	// Basic attack - Light attack, Heavy Attack, Charge Attack
	else if(ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_LightAttack))
	{
		return TryCombatAction(EMVCombatActionTypes::LightAttack);
	}
	else if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyAttack))
	{
		return TryCombatAction(EMVCombatActionTypes::HeavyAttack);
	}
	else if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyChargeAttack))
	{
		return TryCombatAction(EMVCombatActionTypes::HeavyAttack);
	}
	else if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_ChargeAttack))
	{
		if (ShouldSuppressChargeAttackInputForSprint())
		{
			const bool bCanStartSprintHeavyAttack = IsSprintAttackContext()
				&& !bSprintContextualBasicAttackConsumed;
			if (bCanStartSprintHeavyAttack)
			{
				return TryCombatAction(EMVCombatActionTypes::HeavyAttack);
			}

			UE_LOG(LogMVCombatComponent, Log, TEXT("ChargeAttack input suppressed during sprint attack context."));
			return true;
		}

		return TryCombatAction(EMVCombatActionTypes::ChargeAttack);
	}

	return false;
}

bool UMVCombatComponent::IsCombatActionInputTag(const FGameplayTag ActionInputTag) const
{
	return ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_LightAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyChargeAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_ChargeAttack)
		|| ActionInputTag.MatchesTag(MVGameplayTags::Action_Input_Skill);
}

bool UMVCombatComponent::IsHeavyChargeActionInputTag(const FGameplayTag ActionInputTag) const
{
	return ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyChargeAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyAttack);
}




bool UMVCombatComponent::TryBasicAttack(
	EMVCombatActionTypes InActionType,
	int32 ActionIndex,
	FName StartSection)
{
	const FName ActionMapKey = MakeActionTypeMapKey(InActionType);
	FMVSkillEntry* ActionEntry = BasicAttackMap.Find(ActionMapKey);

	if (!ActionEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Action '%s' not found"), *ActionMapKey.ToString());
		return false;
	}

	if (ActionIndex > 0)
	{
		const FName IndexedRowName = MakeIndexedActionRowName(InActionType, ActionIndex);
		FDataTableRowHandle IndexedRowHandle;
		IndexedRowHandle.DataTable = ActionEntry->DataTable.Get();
		IndexedRowHandle.RowName = IndexedRowName;

		if (IndexedRowHandle.DataTable
			&& IndexedRowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(IndexedRowName, TEXT("TryBasicAttack"), false))
		{
			FMVSkillEntry IndexedEntry;
			if (BuildSkillEntryFromRowHandle(IndexedRowHandle, IndexedEntry)
				&& CanConsumeActionCost(IndexedEntry.GetCurrentSkillData()))
			{
				const bool bStarted = TryStartActionWithAbility(IndexedEntry, IndexedRowHandle, StartSection);
				if (bStarted)
				{
					UpdateLastBasicAttackSwingDirection(IndexedRowHandle);
				}
				return bStarted;
			}
		}
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Reset Basic Attack's Index and Start Attack
	if (CurrentTime - LastBasicAttackedTime > ResetBasicAttackTime)
	{
		for (TPair<FName, FMVSkillEntry>& pair : BasicAttackMap)
		{
			FMVSkillEntry& CurrentEntry = pair.Value;
			CurrentEntry.ResetChain();
		}
		ActiveBasicAttackMapKey = NAME_None;
		ClearLastBasicAttackSwingDirection();
	}

	if (!ActiveBasicAttackMapKey.IsNone() && ActiveBasicAttackMapKey != ActionMapKey)
	{
		ResetOtherBasicAttackChains(ActionMapKey);
		ActionEntry->ResetChain();
	}

	const int32 OriginalChainStageIndex = ActionEntry->CurrentChainStageIndex;
	const int32 SelectedChainStageIndex = SelectBasicAttackChainStageForSwing(*ActionEntry);
	ActionEntry->CurrentChainStageIndex = SelectedChainStageIndex;
	const FDataTableRowHandle CurrentRowHandle = ActionEntry->GetCurrentActionRowHandle();

	if (!CanConsumeActionCost(ActionEntry->GetCurrentSkillData()))
	{
		ActionEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		return false;
	}

	UE_LOG(
		LogMVCombatComponent,
		Log,
		TEXT("TryBasicAttack: Action=%s Row=%s ChainStage=%d ChainActive=%s"),
		*ActionMapKey.ToString(),
		*MVCombatRowHandleToString(CurrentRowHandle),
		ActionEntry->CurrentChainStageIndex,
		ActionEntry->bChainActive ? TEXT("true") : TEXT("false"));

	if (ActionEntry->bChainActive)
	{
		if (TryStartActionWithAbility(*ActionEntry, CurrentRowHandle, StartSection))
		{
			UpdateLastBasicAttackSwingDirection(CurrentRowHandle);
			ActionEntry->TryAdvanceChainStage(CurrentTime);
			LastBasicAttackedTime = CurrentTime;
			ActiveBasicAttackMapKey = ActionMapKey;
			return true;
		}
		else
		{
			ActionEntry->CurrentChainStageIndex = OriginalChainStageIndex;
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Failed to send data to ActionComponent for action '%s'"), *ActionMapKey.ToString());
			return false;
		}

	}
	else
	{
		if (TryStartActionWithAbility(*ActionEntry, CurrentRowHandle, StartSection))
		{
			UpdateLastBasicAttackSwingDirection(CurrentRowHandle);
			ActionEntry->ActivateChain(CurrentTime);
			ActionEntry->CurrentChainStageIndex = SelectedChainStageIndex;
			ActionEntry->TryAdvanceChainStage(CurrentTime);
			ResetOtherBasicAttackChains(ActionMapKey);
			ActiveBasicAttackMapKey = ActionMapKey;
		}
		else
		{
			ActionEntry->CurrentChainStageIndex = OriginalChainStageIndex;
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Failed to send data to ActionComponent for action '%s'"), *ActionMapKey.ToString());
			return false;
		}
	}
		LastBasicAttackedTime = CurrentTime;
		return true;

}

bool UMVCombatComponent::TrySkill(
	EMVCombatActionTypes InActionType,
	int32 SkillIndex,
	FName StartSection)
{
	const FName RowName = MakeIndexedActionRowName(InActionType, SkillIndex);

	FMVSkillEntry* ActionEntry = SkillMap.Find(RowName);
	if (!ActionEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryCombatAction - Action '%s' not found"), *RowName.ToString());
		return false;
	}

	if (!CanConsumeActionCost(ActionEntry->GetCurrentSkillData()))
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Chain Is Active --> doing chaining actions
	if (ActionEntry->bChainActive)
	{
		// Input Window is expired
		if (!ActionEntry->IsInputWindowValid(CurrentTime))
		{
			ActionEntry->ResetChain();
			if (!ActionEntry->IsMainCooldownReady(CurrentTime))
			{
				return false;
			}

			if (TryStartActionWithAbility(*ActionEntry, ActionEntry->GetCurrentActionRowHandle(), StartSection))
			{
				ClearLastBasicAttackSwingDirection();
				ActionEntry->ActivateChain(CurrentTime);
				ActionEntry->TryAdvanceChainStage(CurrentTime);
				return true;
			}
			else
			{
				return false;
			}
		}

		// Inter-stage cooldown is not ready
		if (!ActionEntry->IsInterStageCooldownValid(CurrentTime))
		{
			return false;
		}

		if (ActionEntry->CurrentChainStageIndex < ActionEntry->AbilityInstances.Num())
		{

			if (TryStartActionWithAbility(*ActionEntry, ActionEntry->GetCurrentActionRowHandle(), StartSection))
			{
				ClearLastBasicAttackSwingDirection();
				ActionEntry->TryAdvanceChainStage(CurrentTime);
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	// Chain is not Active --> Start Action
	else
	{
		if (!ActionEntry->IsMainCooldownReady(CurrentTime))
		{
			return false;
		}

		if (TryStartActionWithAbility(*ActionEntry, ActionEntry->GetCurrentActionRowHandle(), StartSection))
		{
			ClearLastBasicAttackSwingDirection();
			ActionEntry->ActivateChain(CurrentTime);
			ActionEntry->TryAdvanceChainStage(CurrentTime);
			return true;
		}
		else
		{
			return false;
		}

	}
	return false;
}

void UMVCombatComponent::HandleAbilityEnded(const UMVAbilityBase* EndedAbility)
{
	if (!EndedAbility || !GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	bool bHandledAbility = false;

	for (TPair<FName, FMVSkillEntry>& Pair : BasicAttackMap)
	{
		if (Pair.Value.ContainsAbility(EndedAbility))
		{
			Pair.Value.StartPostAbilityResetWindow(CurrentTime);
			LastBasicAttackedTime = CurrentTime;
			bHandledAbility = true;
			break;
		}
	}

	if (!bHandledAbility)
	{
		for (TPair<FName, FMVSkillEntry>& Pair : SkillMap)
		{
			if (Pair.Value.ContainsAbility(EndedAbility))
			{
				Pair.Value.StartPostAbilityResetWindow(CurrentTime);
				bHandledAbility = true;
				break;
			}
		}
	}

}

void UMVCombatComponent::HandleActionEnded(
	const FName ActionTableName,
	const FName ActionRowName,
	const bool /*bInterrupted*/)
{
	if (HeavyChargeAttackState.bActive
		&& ActionTableName == HeavyChargeAttackState.ChargeActionTableName
		&& ActionRowName == HeavyChargeAttackState.ChargeRowHandle.RowName)
	{
		ResetHeavyChargeAttackState();
	}

	if (!IsCurrentAbilityAction(ActionTableName, ActionRowName))
	{
		return;
	}

	if (CurrentAbilityInstance && CurrentAbilityInstance->bAbilityActive)
	{
		IMVAbilityInterface::Execute_EndAbility(CurrentAbilityInstance);
	}

	CurrentAbilityInstance = nullptr;
	CurrentAbilityActionTableName = NAME_None;
	CurrentAbilityActionRowName = NAME_None;
}

void UMVCombatComponent::RefreshActionMaps()
{

	ResetBasicAttackMap();
	ResetSkillMap();
}

void UMVCombatComponent::ResetBasicAttackMap()
{
	if (BasicAttackMap.Num() != 0)
	{
		BasicAttackMap.Reset();
	}
	ActiveBasicAttackMapKey = NAME_None;
	LastBasicAttackedTime = 0.0;
	bSprintContextualBasicAttackConsumed = false;
	bWasSprintAttackContextActive = false;
	ConsumedDodgeContextActionInstanceId = INDEX_NONE;
	PendingDodgeContextActionInstanceId = INDEX_NONE;
	ClearLastBasicAttackSwingDirection();
	ResetHeavyChargeAttackState();
	
	FMVCombatActionTableInput ChooserInput;
	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;

	TArray<EMVCombatActionTypes> TypeArray;
	TypeArray.Add(EMVCombatActionTypes::LightAttack);
	TypeArray.Add(EMVCombatActionTypes::HeavyAttack);
	TypeArray.Add(EMVCombatActionTypes::ChargeAttack);
	TypeArray.Add(EMVCombatActionTypes::SprintLightAttack);
	TypeArray.Add(EMVCombatActionTypes::SprintHeavyAttack);
	TypeArray.Add(EMVCombatActionTypes::DodgeLightAttack);
	TypeArray.Add(EMVCombatActionTypes::DodgeHeavyAttack);

	for (EMVCombatActionTypes Types : TypeArray)
	{
		ChooserInput.SetActionType(MakeActionTypeGameplayTag(Types));
		const FName ActionMapKey = MakeActionTypeMapKey(Types);

		FDataTableRowHandle StartingRowHandle;
		const bool bResolvedFromChooser = ResolveActionRowHandleFromChooserTable(ChooserInput, ActionMapKey, StartingRowHandle);
		if (bResolvedFromChooser && !MVCombatIsBasicAttackStartRowCandidate(Types, StartingRowHandle.RowName))
		{
			UE_LOG(
				LogMVCombatComponent,
				Warning,
				TEXT("ResetBasicAttackMap: Chooser returned an unexpected start row for action '%s'. Returned=%s. Using fallback row candidates."),
				*ActionMapKey.ToString(),
				*MVCombatRowHandleToString(StartingRowHandle));
			StartingRowHandle = FDataTableRowHandle();
		}

		if (StartingRowHandle.RowName.IsNone() || !StartingRowHandle.DataTable)
		{
			if (!TryMakeFallbackAttackActionRowHandle(Types, StartingRowHandle))
			{
				UE_LOG(LogMVCombatComponent, Log, TEXT("ResetBasicAttackMap: Cannot get row handle for action '%s'"), *ActionMapKey.ToString());
				continue;
			}
		}

		FMVSkillEntry SkillEntry;
		if (!BuildSkillEntryFromRowHandle(StartingRowHandle, SkillEntry))
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent(Reset Basic Attack Map): Failed to build action '%s' from row '%s'"),
				*ActionMapKey.ToString(),
				*StartingRowHandle.RowName.ToString());
			continue;
		}

		BasicAttackMap.Add(ActionMapKey, SkillEntry);
		UE_LOG(
			LogMVCombatComponent,
			Log,
			TEXT("ResetBasicAttackMap: Registered Action=%s Start=%s Chain=%s"),
			*ActionMapKey.ToString(),
			*MVCombatRowHandleToString(StartingRowHandle),
			*MVCombatSkillRowNamesToString(SkillEntry.SkillRowNames));
	}

}

void UMVCombatComponent::ResetSkillMap()
{
	// If AnyValue remains in maps, clear map
	if (SkillMap.Num() != 0)
	{
		SkillMap.Reset();
	}

	// Make ChooserTable function input 
	bool DataTableSearchResult = false;
	FMVCombatActionTableInput ChooserInput;
	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;
	ChooserInput.SetActionType(MakeActionTypeGameplayTag(EMVCombatActionTypes::Skill));

	// Find DataTable using ChooserTable input
	UDataTable* CurrentDT = GetDataTableFromChooserTable(ChooserInput, DataTableSearchResult);
	bool bUsingFallbackAttackActionTable = false;

	if (!DataTableSearchResult || !CurrentDT)
	{
		CurrentDT = LoadFallbackAttackActionTable();
		bUsingFallbackAttackActionTable = CurrentDT != nullptr;
		if (!CurrentDT)
		{
			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map): Cannot Get DataTable"));
			return;
		}
	}

	if (!CurrentDT->GetRowStruct() || !CurrentDT->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct()))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent(Reset Skill Map): DataTable '%s' has invalid row struct '%s'"),
			*CurrentDT->GetName(),
			CurrentDT->GetRowStruct() ? *CurrentDT->GetRowStruct()->GetName() : TEXT("None"));
		return;
	}

	TSet<FName> ProcessedRowNames;

	TArray<FName> RowNames = CurrentDT->GetRowNames();
	RowNames.Sort(
		[](const FName& Left, const FName& Right)
		{
			return Left.ToString() < Right.ToString();
		});

	int32 SkillIndex = 0;
	for (const FName RowName : RowNames)
	{
		FMVSkillDataTableColumn* RowData = CurrentDT->FindRow<FMVSkillDataTableColumn>(
			RowName,
			TEXT("ResetSkillMap"),
			false);

		if (!RowData || !RowData->AbilityReference)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent(Reset Skill Map): Invalid row data or ability reference for '%s'"), *RowName.ToString());
			continue;
		}

		if (bUsingFallbackAttackActionTable && !MVCombatIsSkillStartRowName(RowName))
		{
			continue;
		}

		// Chained Skill will add previous when first skill(first Chain skill) add to the map
		//if (SkillMap.Contains(It.Key))
		//{
		//	continue;
		//}
		if (ProcessedRowNames.Contains(RowName))
		{
			continue;
		}

		FDataTableRowHandle StartingRowHandle;
		StartingRowHandle.DataTable = CurrentDT;
		StartingRowHandle.RowName = RowName;

		FMVSkillEntry SkillEntry;
		if (!BuildSkillEntryFromRowHandle(StartingRowHandle, SkillEntry))
		{
			continue;
		}

		for(const FName& SkillRowName : SkillEntry.SkillRowNames)
		{
			ProcessedRowNames.Add(SkillRowName);
		}

		SkillMap.Add(MVCombatMakeSkillMapKey(SkillIndex), SkillEntry);
		++SkillIndex;
	}

}

void UMVCombatComponent::ChangeWeapon(EMVEquippedStyle NewStyle)
{
	// Change Current Weapon Style
	CurrentWeaponStyle = NewStyle;

	// Refresh Attack Index Maps
	RefreshActionMaps();
}

FMVSkillDataTableColumn UMVCombatComponent::GetDataTableRowFromChooserTable_Implementation(const FMVCombatActionTableInput& ChooserInput, const FName& RowName, bool& OutResult)
{
	FMVSkillDataTableColumn OutRow;

	FDataTableRowHandle RowHandle;
	if (!ResolveActionRowHandleFromChooserTable(ChooserInput, RowName, RowHandle))
	{
		OutResult = false;
		return OutRow;
	}

	// Find Row
	FMVSkillDataTableColumn* Data = RowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(RowHandle.RowName, TEXT("DataTableRow"));
	
	if (Data)
	{
		OutRow = *Data;
		OutResult = true;
	}
	else
	{
		OutResult = false;
	}

	return OutRow;
}

bool UMVCombatComponent::BuildSkillEntryFromRowHandle(const FDataTableRowHandle& StartingRowHandle, FMVSkillEntry& OutEntry)
{
	if (!IsValidSkillActionRowHandle(StartingRowHandle, TEXT("BuildSkillEntryFromRowHandle")))
	{
		return false;
	}

	OutEntry = FMVSkillEntry();
	OutEntry.DataTable = const_cast<UDataTable*>(StartingRowHandle.DataTable.Get());

	const UDataTable* CurrentDT = StartingRowHandle.DataTable.Get();
	FName CurrentName = StartingRowHandle.RowName;
	int32 ChainLength = 0;
	int32 MaxChainLength = 100;

	while (!CurrentName.IsNone() && ChainLength < MaxChainLength)
	{
		FMVSkillDataTableColumn* RowData = CurrentDT->FindRow<FMVSkillDataTableColumn>(CurrentName, TEXT("BuildSkillEntryFromRowHandle"));
		if (!RowData || !RowData->AbilityReference)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::BuildSkillEntryFromRowHandle - Invalid row '%s'"), *CurrentName.ToString());
			break;
		}

		if (ChainLength == 0)
		{
			OutEntry.MainCooldownDuration = RowData->CooldownDuration;
		}

		// Create ability instance
		UMVAbilityBase* AbilityInstance = NewObject<UMVAbilityBase>(this, RowData->AbilityReference);
		if (AbilityInstance)
		{
			AbilityInstance->SetOwner(this);
			AbilityInstance->InitAbility(*RowData);
			OutEntry.AbilityInstances.Add(AbilityInstance);
			OutEntry.SkillRowNames.Add(CurrentName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::BuildSkillEntryFromRowHandle - Failed to create ability for row '%s'"), *CurrentName.ToString());
			break;
		}

		// Move to next row in chain
		if (RowData->bIsChained && !RowData->NextChainName.IsNone())
		{
			CurrentName = RowData->NextChainName;
		}
		else
		{
			CurrentName = NAME_None;
		}

		ChainLength++;
	}

	if (!CurrentName.IsNone() && ChainLength >= MaxChainLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::BuildSkillEntryFromRowHandle - Chain exceeded max length. StartRow='%s'"), *StartingRowHandle.RowName.ToString());
	}

	OutEntry.bIsChained = OutEntry.AbilityInstances.Num() > 1;
	return OutEntry.AbilityInstances.Num() > 0;
}

bool UMVCombatComponent::GetActionRowHandleFromChooserTable(
	const FMVCombatActionTableInput& ChooserInput,
	const FName FallbackRowName,
	FDataTableRowHandle& OutRowHandle) const
{
	OutRowHandle = FDataTableRowHandle();

	if (!AttackChooserTable.IsValid())
	{
		return false;
	}

	UChooserTable* ChooserTable = Cast<UChooserTable>(AttackChooserTable.TryLoad());
	if (!ChooserTable)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent::GetActionRowHandleFromChooserTable - Chooser failed to load. Path=%s."),
			*AttackChooserTable.ToString());
		return false;
	}

	FMVCombatActionTableInput MutableChooserInput = ChooserInput;
	MutableChooserInput.RefreshActionTypeTags();
	FGameplayTag ActionTypeTag = MutableChooserInput.ActionType;
	FGameplayTagContainer ActionTypeTags = MutableChooserInput.ActionTypeTags;
	FMVAttackActionRowHandle ChooserAttackActionRowHandle;
	ChooserAttackActionRowHandle.Reset();

	FChooserEvaluationContext ChooserContext;
	ChooserContext.AddStructParam(MutableChooserInput);
	ChooserContext.AddStructParam(ChooserAttackActionRowHandle);
	ChooserContext.AddStructParam(ActionTypeTag);
	ChooserContext.AddStructParam(ActionTypeTags);

	TSoftObjectPtr<UObject> SelectedObject;
	UChooserTable::EvaluateChooser(
		ChooserContext,
		ChooserTable,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));

	if (ChooserAttackActionRowHandle.ActionRow.DataTable)
	{
		OutRowHandle = ChooserAttackActionRowHandle.ActionRow;
		if (OutRowHandle.RowName.IsNone())
		{
			OutRowHandle.RowName = FallbackRowName;
		}
		return !OutRowHandle.RowName.IsNone();
	}

	UObject* ResolvedObject = SelectedObject.IsValid()
		? SelectedObject.Get()
		: SelectedObject.LoadSynchronous();
	UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
	if (!SelectedDataTable)
	{
		return false;
	}

	OutRowHandle.DataTable = SelectedDataTable;
	OutRowHandle.RowName = FallbackRowName;
	return !OutRowHandle.RowName.IsNone();
}

bool UMVCombatComponent::ResolveActionRowHandleFromChooserTable(
	const FMVCombatActionTableInput& ChooserInput,
	const FName FallbackRowName,
	FDataTableRowHandle& OutRowHandle)
{
	if (GetActionRowHandleFromChooserTable(ChooserInput, FallbackRowName, OutRowHandle))
	{
		return true;
	}

	return false;
}

UDataTable* UMVCombatComponent::LoadFallbackAttackActionTable() const
{
	if (!FallbackAttackActionTable.IsValid())
	{
		return nullptr;
	}

	UDataTable* DataTable = Cast<UDataTable>(FallbackAttackActionTable.TryLoad());
	if (!DataTable)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent::LoadFallbackAttackActionTable - Fallback table failed to load. Path=%s."),
			*FallbackAttackActionTable.ToString());
		return nullptr;
	}

	if (!DataTable->GetRowStruct() || !DataTable->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct()))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent::LoadFallbackAttackActionTable - DataTable '%s' has invalid row struct '%s'"),
			*DataTable->GetName(),
			DataTable->GetRowStruct() ? *DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return nullptr;
	}

	return DataTable;
}

bool UMVCombatComponent::TryMakeFallbackAttackActionRowHandle(
	const EMVCombatActionTypes ActionType,
	FDataTableRowHandle& OutRowHandle) const
{
	OutRowHandle = FDataTableRowHandle();

	UDataTable* DataTable = LoadFallbackAttackActionTable();
	if (!DataTable)
	{
		return false;
	}

	for (const FName CandidateRowName : MVCombatMakeFallbackAttackRowCandidates(ActionType))
	{
		if (DataTable->FindRow<FMVSkillDataTableColumn>(
			CandidateRowName,
			TEXT("TryMakeFallbackAttackActionRowHandle"),
			false))
		{
			OutRowHandle.DataTable = DataTable;
			OutRowHandle.RowName = CandidateRowName;
			return true;
		}
	}

	return false;
}

bool UMVCombatComponent::IsValidSkillActionRowHandle(const FDataTableRowHandle& RowHandle, const TCHAR* Context) const
{
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent::%s - Invalid row handle. DataTable=%s RowName=%s"),
			Context,
			*GetNameSafe(RowHandle.DataTable),
			*RowHandle.RowName.ToString());
		return false;
	}

	if (!RowHandle.DataTable->GetRowStruct() || !RowHandle.DataTable->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct()))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("MVCombatComponent::%s - DataTable '%s' has invalid row struct '%s'"),
			Context,
			*RowHandle.DataTable->GetName(),
			RowHandle.DataTable->GetRowStruct() ? *RowHandle.DataTable->GetRowStruct()->GetName() : TEXT("None"));
		return false;
	}

	return true;
}

bool UMVCombatComponent::IsCurrentAbilityAction(
	const FName ActionTableName,
	const FName ActionRowName) const
{
	return !CurrentAbilityActionTableName.IsNone()
		&& !CurrentAbilityActionRowName.IsNone()
		&& ActionTableName == CurrentAbilityActionTableName
		&& ActionRowName == CurrentAbilityActionRowName;
}

FName UMVCombatComponent::MakeActionTypeMapKey(const EMVCombatActionTypes ActionType) const
{
	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
	return EnumPtr
		? FName(*EnumPtr->GetNameStringByValue((int64)ActionType))
		: NAME_None;
}

FName UMVCombatComponent::MakeIndexedActionRowName(
	const EMVCombatActionTypes ActionType,
	const int32 ActionIndex) const
{
	const FName TypeName = MakeActionTypeMapKey(ActionType);
	return TypeName.IsNone()
		? NAME_None
		: FName(*FString::Printf(TEXT("%s%d"), *TypeName.ToString(), ActionIndex));
}

FGameplayTag UMVCombatComponent::MakeActionTypeGameplayTag(const EMVCombatActionTypes ActionType) const
{
	switch (ActionType)
	{
	case EMVCombatActionTypes::LightAttack:
		return MVGameplayTags::Action_Combat_LightAttack;
	case EMVCombatActionTypes::HeavyAttack:
		return MVGameplayTags::Action_Combat_HeavyAttack;
	case EMVCombatActionTypes::ChargeAttack:
		return MVGameplayTags::Action_Combat_ChargeAttack;
	case EMVCombatActionTypes::Skill:
		return MVGameplayTags::Action_Combat_Skill;
	case EMVCombatActionTypes::SprintLightAttack:
		return MVGameplayTags::Action_Combat_SprintLightAttack;
	case EMVCombatActionTypes::SprintHeavyAttack:
		return MVGameplayTags::Action_Combat_SprintHeavyAttack;
	case EMVCombatActionTypes::DodgeLightAttack:
		return MVGameplayTags::Action_Combat_DodgeLightAttack;
	case EMVCombatActionTypes::DodgeHeavyAttack:
		return MVGameplayTags::Action_Combat_DodgeHeavyAttack;
	default:
		return FGameplayTag();
	}
}

bool UMVCombatComponent::TryStartActionWithAbility(
	FMVSkillEntry& ActionEntry,
	const FDataTableRowHandle& RowHandle,
	FName StartSection,
	const bool bForceTransition,
	const float TransitionBlendOutTime)
{
	if (!IsValidSkillActionRowHandle(RowHandle, TEXT("TryStartActionWithAbility")))
	{
		return false;
	}

	AMVCharacterBase* Owner = Cast<AMVCharacterBase>(GetOwner());
	if (!Owner)
	{
		return false;
	}
	
	UMVActionComponent* ActionComponent = Owner->ActionComponent;
	if (!ActionComponent)
	{
		return false;
	}

	UMVInputManagerComponent* InputManager = Owner->InputManagerComponent;
	const bool bRecoveryOpen = InputManager ? InputManager->IsRecoveryEscapeWindowOpen() : true;
	const bool bIsActionRunning = ActionComponent->IsActionRunning();
	const bool bCanInterrupt = ActionComponent->CanInterruptActiveAction();

	const auto PrepareCurrentAbility =
		[this, &ActionEntry, &RowHandle]() -> UMVAbilityBase*
		{
			UMVAbilityBase* NextAbility = ActionEntry.GetCurrentAbility();
			PreviousAbilityInstance = CurrentAbilityInstance;
			if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
			{
				IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
			}

			CurrentAbilityInstance = NextAbility;
			CurrentAbilityActionTableName = MVCombatActionTableNameFromDataTable(RowHandle.DataTable);
			CurrentAbilityActionRowName = RowHandle.RowName;
			if (CurrentAbilityInstance)
			{
				CurrentAbilityInstance->PrepareAbilityExecution();
			}

			return NextAbility;
		};

	const auto ClearPreparedAbilityOnFailure =
		[this](const UMVAbilityBase* PreparedAbility)
		{
			if (CurrentAbilityInstance.Get() == PreparedAbility)
			{
				CurrentAbilityInstance = nullptr;
				CurrentAbilityActionTableName = NAME_None;
				CurrentAbilityActionRowName = NAME_None;
			}
		};

	// If an action is running, only allow transition when recovery is open or action is interruptible.
	if (bIsActionRunning)
	{
		if (bForceTransition || (bRecoveryOpen && bCanInterrupt))
		{
			const UMVAbilityBase* PreparedAbility = PrepareCurrentAbility();
			const bool bStarted = ActionComponent->TryTransitionActionFromRowHandle(
				RowHandle,
				StartSection,
				FMath::Max(0.0f, TransitionBlendOutTime));
			if (!bStarted)
			{
				ClearPreparedAbilityOnFailure(PreparedAbility);
			}
			return bStarted;
		}
		else
		{
			// Cannot transition now -> keep buffered input (return false)
			return false;
		}
	}
	else
	{
		// No active action -> start normally
		const UMVAbilityBase* PreparedAbility = PrepareCurrentAbility();
		const bool bStarted = ActionComponent->TryStartActionFromRowHandle(RowHandle, StartSection);
		if (!bStarted)
		{
			ClearPreparedAbilityOnFailure(PreparedAbility);
		}
		return bStarted;
	}
}

bool UMVCombatComponent::CanConsumeActionCost(const FMVSkillDataTableColumn* SkillData) const
{
	if (!SkillData)
	{
		return false;
	}

	if (!StatComponent)
	{
		return false;
	}

	if (SkillData->StaminaCost > 0.0f && !StatComponent->HasAnyStamina())
	{
		return false;
	}
	
	if (!StatComponent->HasMP(SkillData->MpCost))
	{
		return false;
	}

	return true;
}

bool UMVCombatComponent::IsBasicAttackActionType(const EMVCombatActionTypes ActionType) const
{
	return ActionType == EMVCombatActionTypes::LightAttack
		|| ActionType == EMVCombatActionTypes::HeavyAttack
		|| ActionType == EMVCombatActionTypes::ChargeAttack
		|| ActionType == EMVCombatActionTypes::SprintLightAttack
		|| ActionType == EMVCombatActionTypes::SprintHeavyAttack
		|| ActionType == EMVCombatActionTypes::DodgeLightAttack
		|| ActionType == EMVCombatActionTypes::DodgeHeavyAttack;
}

int32 UMVCombatComponent::SelectBasicAttackChainStageForSwing(const FMVSkillEntry& ActionEntry) const
{
	const int32 CurrentStageIndex = ActionEntry.SkillRowNames.IsValidIndex(ActionEntry.CurrentChainStageIndex)
		? ActionEntry.CurrentChainStageIndex
		: 0;
	const EMVAttackSwingDirection DesiredSwingDirection =
		MVCombatGetOppositeSwingDirection(LastBasicAttackSwingDirection);
	if (DesiredSwingDirection == EMVAttackSwingDirection::None || !ActionEntry.DataTable)
	{
		return CurrentStageIndex;
	}

	for (int32 StageIndex = CurrentStageIndex; StageIndex < ActionEntry.SkillRowNames.Num(); ++StageIndex)
	{
		const FMVSkillDataTableColumn* SkillData = GetBasicAttackSkillDataAtStage(ActionEntry, StageIndex);
		if (SkillData && SkillData->SwingDirection == DesiredSwingDirection)
		{
			return StageIndex;
		}
	}

	return CurrentStageIndex;
}

const FMVSkillDataTableColumn* UMVCombatComponent::GetBasicAttackSkillDataAtStage(
	const FMVSkillEntry& ActionEntry,
	const int32 ChainStageIndex) const
{
	if (!ActionEntry.DataTable || !ActionEntry.SkillRowNames.IsValidIndex(ChainStageIndex))
	{
		return nullptr;
	}

	return ActionEntry.DataTable->FindRow<FMVSkillDataTableColumn>(
		ActionEntry.SkillRowNames[ChainStageIndex],
		TEXT("GetBasicAttackSkillDataAtStage"),
		false);
}

void UMVCombatComponent::UpdateLastBasicAttackSwingDirection(const FDataTableRowHandle& RowHandle)
{
	LastBasicAttackSwingDirection = EMVAttackSwingDirection::None;
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return;
	}

	const FMVSkillDataTableColumn* SkillData = RowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(
		RowHandle.RowName,
		TEXT("UpdateLastBasicAttackSwingDirection"),
		false);
	if (SkillData)
	{
		LastBasicAttackSwingDirection = SkillData->SwingDirection;
	}
}

void UMVCombatComponent::ClearLastBasicAttackSwingDirection()
{
	LastBasicAttackSwingDirection = EMVAttackSwingDirection::None;
}

void UMVCombatComponent::ResetOtherBasicAttackChains(const FName ExceptActionMapKey)
{
	for (TPair<FName, FMVSkillEntry>& Pair : BasicAttackMap)
	{
		if (Pair.Key != ExceptActionMapKey)
		{
			Pair.Value.ResetChain();
		}
	}
}

void UMVCombatComponent::MarkBasicAttackChainStarted(
	FMVSkillEntry& ActionEntry,
	const FName ActionMapKey,
	const int32 StartedChainStageIndex,
	const float CurrentTime)
{
	const int32 ValidStartedChainStageIndex = ActionEntry.SkillRowNames.IsValidIndex(StartedChainStageIndex)
		? StartedChainStageIndex
		: 0;

	ActionEntry.CurrentChainStageIndex = ValidStartedChainStageIndex;
	if (!ActionEntry.bChainActive)
	{
		ActionEntry.ActivateChain(CurrentTime);
		ActionEntry.CurrentChainStageIndex = ValidStartedChainStageIndex;
	}

	ActionEntry.TryAdvanceChainStage(CurrentTime);
	ResetOtherBasicAttackChains(ActionMapKey);
	LastBasicAttackedTime = CurrentTime;
	ActiveBasicAttackMapKey = ActionMapKey;
}

bool UMVCombatComponent::IsHeavyChargeBasicAttackMapKey(const FName ActionMapKey) const
{
	return ActionMapKey == MakeActionTypeMapKey(EMVCombatActionTypes::HeavyAttack)
		|| ActionMapKey == MakeActionTypeMapKey(EMVCombatActionTypes::ChargeAttack);
}

int32 UMVCombatComponent::ResolveHeavyChargeStartChainStageIndex(const FMVSkillEntry& ChargeEntry) const
{
	int32 ChainStageIndex = ChargeEntry.SkillRowNames.IsValidIndex(ChargeEntry.CurrentChainStageIndex)
		? ChargeEntry.CurrentChainStageIndex
		: 0;

	if (!IsHeavyChargeBasicAttackMapKey(ActiveBasicAttackMapKey))
	{
		return ChainStageIndex;
	}

	const FMVSkillEntry* ActiveEntry = BasicAttackMap.Find(ActiveBasicAttackMapKey);
	if (ActiveEntry && ActiveEntry->SkillRowNames.IsValidIndex(ActiveEntry->CurrentChainStageIndex))
	{
		ChainStageIndex = ActiveEntry->CurrentChainStageIndex;
	}

	return ChainStageIndex;
}

int32 UMVCombatComponent::ResolveHeavyChargeEarlyReleaseChainStageIndex(const FMVSkillEntry& HeavyEntry) const
{
	const FName HeavyActionMapKey = MakeActionTypeMapKey(EMVCombatActionTypes::HeavyAttack);
	const bool bContinueHeavyChain = ActiveBasicAttackMapKey == HeavyActionMapKey
		&& HeavyEntry.bChainActive;
	const bool bUseChargeChainStage = !bContinueHeavyChain
		&& HeavyEntry.SkillRowNames.IsValidIndex(HeavyChargeAttackState.ChargeChainStageIndex);
	int32 CurrentStageIndex = 0;
	if (bContinueHeavyChain && HeavyEntry.SkillRowNames.IsValidIndex(HeavyEntry.CurrentChainStageIndex))
	{
		CurrentStageIndex = HeavyEntry.CurrentChainStageIndex;
	}
	else if (bUseChargeChainStage)
	{
		CurrentStageIndex = HeavyChargeAttackState.ChargeChainStageIndex;
	}

	const EMVAttackSwingDirection DesiredSwingDirection =
		MVCombatGetOppositeSwingDirection(LastBasicAttackSwingDirection);
	if (DesiredSwingDirection == EMVAttackSwingDirection::None || !HeavyEntry.DataTable)
	{
		return CurrentStageIndex;
	}

	for (int32 StageIndex = CurrentStageIndex; StageIndex < HeavyEntry.SkillRowNames.Num(); ++StageIndex)
	{
		const FMVSkillDataTableColumn* SkillData = GetBasicAttackSkillDataAtStage(HeavyEntry, StageIndex);
		if (SkillData && SkillData->SwingDirection == DesiredSwingDirection)
		{
			return StageIndex;
		}
	}

	if (!bContinueHeavyChain && !bUseChargeChainStage)
	{
		for (int32 StageIndex = 0; StageIndex < CurrentStageIndex; ++StageIndex)
		{
			const FMVSkillDataTableColumn* SkillData = GetBasicAttackSkillDataAtStage(HeavyEntry, StageIndex);
			if (SkillData && SkillData->SwingDirection == DesiredSwingDirection)
			{
				return StageIndex;
			}
		}
	}

	return CurrentStageIndex;
}

bool UMVCombatComponent::TryBeginHeavyChargeAttack(const float /*HeldSeconds*/)
{
	if (HeavyChargeAttackState.bActive)
	{
		return true;
	}

	UpdateContextualBasicAttackResets();

	if (IsDodgeAttackContext())
	{
		return TryCombatAction(EMVCombatActionTypes::HeavyAttack);
	}

	if (ShouldSuppressChargeAttackInputForSprint())
	{
		const bool bCanStartSprintHeavyAttack = IsSprintAttackContext()
			&& !bSprintContextualBasicAttackConsumed;
		return bCanStartSprintHeavyAttack
			? TryCombatAction(EMVCombatActionTypes::HeavyAttack)
			: true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FMVSkillEntry* ChargeEntry = BasicAttackMap.Find(MakeActionTypeMapKey(EMVCombatActionTypes::ChargeAttack));
	if (!ChargeEntry)
	{
		return false;
	}
	FMVSkillEntry* HeavyEntry = BasicAttackMap.Find(MakeActionTypeMapKey(EMVCombatActionTypes::HeavyAttack));

	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastBasicAttackedTime > ResetBasicAttackTime)
	{
		for (TPair<FName, FMVSkillEntry>& Pair : BasicAttackMap)
		{
			Pair.Value.ResetChain();
		}
		ActiveBasicAttackMapKey = NAME_None;
		ClearLastBasicAttackSwingDirection();
	}

	const FName ChargeActionMapKey = MakeActionTypeMapKey(EMVCombatActionTypes::ChargeAttack);
	const bool bContinueHeavyChargeChain = IsHeavyChargeBasicAttackMapKey(ActiveBasicAttackMapKey);
	if (!ActiveBasicAttackMapKey.IsNone() && !bContinueHeavyChargeChain)
	{
		ResetOtherBasicAttackChains(ChargeActionMapKey);
		ChargeEntry->ResetChain();
		if (HeavyEntry)
		{
			HeavyEntry->ResetChain();
		}
	}

	const int32 OriginalChainStageIndex = ChargeEntry->CurrentChainStageIndex;
	ChargeEntry->CurrentChainStageIndex = ResolveHeavyChargeStartChainStageIndex(*ChargeEntry);
	const int32 SelectedChainStageIndex = SelectBasicAttackChainStageForSwing(*ChargeEntry);
	ChargeEntry->CurrentChainStageIndex = SelectedChainStageIndex;
	const FDataTableRowHandle ChargeRowHandle = ChargeEntry->GetCurrentActionRowHandle();
	const FMVSkillDataTableColumn* ChargeSkillData = ChargeEntry->GetCurrentSkillData();
	if (!CanConsumeActionCost(ChargeSkillData))
	{
		ChargeEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		return false;
	}

	const bool bStarted = TryStartActionWithAbility(*ChargeEntry, ChargeRowHandle, HeavyChargeStartSection);
	if (!bStarted)
	{
		ChargeEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		return false;
	}

	HeavyChargeAttackState = FMVCombatHeavyChargeAttackRuntimeState();
	HeavyChargeAttackState.bActive = true;
	HeavyChargeAttackState.StartedWorldTime = CurrentTime;
	HeavyChargeAttackState.CommitTime = ResolveHeavyChargeCommitTime(ChargeSkillData);
	HeavyChargeAttackState.ChargeActionTableName = MVCombatActionTableNameFromDataTable(ChargeRowHandle.DataTable);
	HeavyChargeAttackState.ChargeActionMapKey = ChargeActionMapKey;
	HeavyChargeAttackState.ChargeChainStageIndex = SelectedChainStageIndex;
	HeavyChargeAttackState.ChargeRowHandle = ChargeRowHandle;
	return true;
}

bool UMVCombatComponent::TryUpdateHeavyChargeAttack(const float HeldSeconds)
{
	if (!HeavyChargeAttackState.bActive)
	{
		return false;
	}

	if (!IsHeavyChargeActionRunning())
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	TryCommitHeavyChargeAttack(HeldSeconds);
	return true;
}

bool UMVCombatComponent::TryReleaseHeavyChargeAttack(const float HeldSeconds, const bool bCanceled)
{
	if (!HeavyChargeAttackState.bActive)
	{
		return false;
	}

	const float EffectiveHeldSeconds = GetWorld()
		? FMath::Max(0.0f, GetWorld()->GetTimeSeconds() - HeavyChargeAttackState.StartedWorldTime)
		: FMath::Max(0.0f, HeldSeconds);
	const bool bShouldCommitCharge = HeavyChargeAttackState.bCommitted
		|| (!bCanceled && EffectiveHeldSeconds >= HeavyChargeAttackState.CommitTime);

	if (bShouldCommitCharge)
	{
		TryCommitHeavyChargeAttack(EffectiveHeldSeconds);
		ResetHeavyChargeAttackState();
		return true;
	}

	return TryEarlyReleaseHeavyChargeAttack(EffectiveHeldSeconds);
}

bool UMVCombatComponent::TryCommitHeavyChargeAttack(const float /*HeldSeconds*/)
{
	if (!HeavyChargeAttackState.bActive)
	{
		return false;
	}

	if (HeavyChargeAttackState.bCommitted)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float EffectiveHeldSeconds =
		FMath::Max(0.0f, World->GetTimeSeconds() - HeavyChargeAttackState.StartedWorldTime);
	if (EffectiveHeldSeconds < HeavyChargeAttackState.CommitTime)
	{
		return false;
	}

	FMVSkillEntry* ChargeEntry = BasicAttackMap.Find(HeavyChargeAttackState.ChargeActionMapKey);
	if (!ChargeEntry || !IsHeavyChargeActionRunning())
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	MarkBasicAttackChainStarted(
		*ChargeEntry,
		HeavyChargeAttackState.ChargeActionMapKey,
		HeavyChargeAttackState.ChargeChainStageIndex,
		World->GetTimeSeconds());
	UpdateLastBasicAttackSwingDirection(HeavyChargeAttackState.ChargeRowHandle);
	HeavyChargeAttackState.bCommitted = true;
	return true;
}

bool UMVCombatComponent::TryEarlyReleaseHeavyChargeAttack(const float /*HeldSeconds*/)
{
	if (!HeavyChargeAttackState.bActive || !IsHeavyChargeActionRunning())
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	FMVSkillEntry* HeavyEntry = nullptr;
	FDataTableRowHandle HeavyRowHandle;
	int32 HeavyChainStageIndex = INDEX_NONE;
	if (!ResolveHeavyChargeEarlyReleaseRow(HeavyEntry, HeavyRowHandle, HeavyChainStageIndex)
		|| !HeavyEntry)
	{
		ResetHeavyChargeAttackState();
		return false;
	}

	const FMVSkillDataTableColumn* ChargeSkillData = HeavyChargeAttackState.ChargeRowHandle.DataTable
		? HeavyChargeAttackState.ChargeRowHandle.DataTable->FindRow<FMVSkillDataTableColumn>(
			HeavyChargeAttackState.ChargeRowHandle.RowName,
			TEXT("TryEarlyReleaseHeavyChargeAttack"),
			false)
		: nullptr;
	const FName EarlyReleaseStartSection = ChargeSkillData && !ChargeSkillData->EarlyReleaseStartSection.IsNone()
		? ChargeSkillData->EarlyReleaseStartSection
		: FName(TEXT("Attack"));
	const float EarlyReleaseBlendOutTime = ChargeSkillData
		? FMath::Max(0.0f, ChargeSkillData->EarlyReleaseBlendOutTime)
		: 0.1f;

	const int32 OriginalChainStageIndex = HeavyEntry->CurrentChainStageIndex;
	HeavyEntry->CurrentChainStageIndex = HeavyChainStageIndex;
	if (!CanConsumeActionCost(HeavyEntry->GetCurrentSkillData()))
	{
		HeavyEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		ResetHeavyChargeAttackState();
		return false;
	}

	const bool bStarted = TryStartActionWithAbility(
		*HeavyEntry,
		HeavyRowHandle,
		EarlyReleaseStartSection,
		true,
		EarlyReleaseBlendOutTime);
	if (!bStarted)
	{
		HeavyEntry->CurrentChainStageIndex = OriginalChainStageIndex;
		ResetHeavyChargeAttackState();
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		const FName HeavyActionMapKey = MakeActionTypeMapKey(EMVCombatActionTypes::HeavyAttack);
		MarkBasicAttackChainStarted(
			*HeavyEntry,
			HeavyActionMapKey,
			HeavyChainStageIndex,
			World->GetTimeSeconds());
	}
	UpdateLastBasicAttackSwingDirection(HeavyRowHandle);
	ResetHeavyChargeAttackState();
	return true;
}

bool UMVCombatComponent::IsHeavyChargeActionRunning() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	return HeavyChargeAttackState.bActive
		&& ActionComponent
		&& ActionComponent->IsActionRunning()
		&& ActionComponent->GetActiveActionTableName() == HeavyChargeAttackState.ChargeActionTableName
		&& ActionComponent->GetActiveActionRowName() == HeavyChargeAttackState.ChargeRowHandle.RowName;
}

float UMVCombatComponent::ResolveHeavyChargeCommitTime(const FMVSkillDataTableColumn* ChargeSkillData) const
{
	if (ChargeSkillData && ChargeSkillData->ChargeCommitTime > 0.0f)
	{
		return ChargeSkillData->ChargeCommitTime;
	}

	return FMath::Max(0.0f, DefaultHeavyChargeCommitTime);
}

bool UMVCombatComponent::ResolveHeavyChargeEarlyReleaseRow(
	FMVSkillEntry*& OutHeavyEntry,
	FDataTableRowHandle& OutHeavyRowHandle,
	int32& OutHeavyChainStageIndex)
{
	OutHeavyEntry = nullptr;
	OutHeavyRowHandle = FDataTableRowHandle();
	OutHeavyChainStageIndex = INDEX_NONE;

	const FName HeavyActionMapKey = MakeActionTypeMapKey(EMVCombatActionTypes::HeavyAttack);
	OutHeavyEntry = BasicAttackMap.Find(HeavyActionMapKey);
	if (!OutHeavyEntry || !OutHeavyEntry->DataTable)
	{
		return false;
	}

	int32 ChainStageIndex = ResolveHeavyChargeEarlyReleaseChainStageIndex(*OutHeavyEntry);
	if (!OutHeavyEntry->SkillRowNames.IsValidIndex(ChainStageIndex))
	{
		ChainStageIndex = OutHeavyEntry->SkillRowNames.IsValidIndex(0) ? 0 : INDEX_NONE;
	}

	if (ChainStageIndex == INDEX_NONE)
	{
		return false;
	}

	OutHeavyChainStageIndex = ChainStageIndex;
	OutHeavyRowHandle.DataTable = OutHeavyEntry->DataTable.Get();
	OutHeavyRowHandle.RowName = OutHeavyEntry->SkillRowNames[ChainStageIndex];
	return !OutHeavyRowHandle.RowName.IsNone();
}

void UMVCombatComponent::ResetHeavyChargeAttackState()
{
	HeavyChargeAttackState = FMVCombatHeavyChargeAttackRuntimeState();
}

EMVCombatActionTypes UMVCombatComponent::ResolveContextualBasicAttackActionType(
	const EMVCombatActionTypes RequestedActionType)
{
	PendingDodgeContextActionInstanceId = INDEX_NONE;

	const bool bRequestedLightAttack = RequestedActionType == EMVCombatActionTypes::LightAttack;
	const bool bRequestedHeavyAttack = RequestedActionType == EMVCombatActionTypes::HeavyAttack;
	if (!bRequestedLightAttack && !bRequestedHeavyAttack)
	{
		return RequestedActionType;
	}

	const int32 DodgeContextActionInstanceId = GetDodgeAttackContextInstanceId();
	const bool bSprintAttackContext = IsSprintAttackContext();
	if (DodgeContextActionInstanceId != INDEX_NONE
		&& DodgeContextActionInstanceId != ConsumedDodgeContextActionInstanceId)
	{
		PendingDodgeContextActionInstanceId = DodgeContextActionInstanceId;
		return bRequestedLightAttack
			? EMVCombatActionTypes::DodgeLightAttack
			: EMVCombatActionTypes::DodgeHeavyAttack;
	}

	if (bSprintAttackContext && !bSprintContextualBasicAttackConsumed)
	{
		return bRequestedLightAttack
			? EMVCombatActionTypes::SprintLightAttack
			: EMVCombatActionTypes::SprintHeavyAttack;
	}

	return RequestedActionType;
}

void UMVCombatComponent::MarkContextualBasicAttackStarted(const EMVCombatActionTypes StartedActionType)
{
	if (MVCombatIsSprintContextualBasicAttackActionType(StartedActionType))
	{
		bSprintContextualBasicAttackConsumed = true;
		return;
	}

	if (MVCombatIsDodgeContextualBasicAttackActionType(StartedActionType)
		&& PendingDodgeContextActionInstanceId != INDEX_NONE)
	{
		ConsumedDodgeContextActionInstanceId = PendingDodgeContextActionInstanceId;
		PendingDodgeContextActionInstanceId = INDEX_NONE;
	}
}

void UMVCombatComponent::UpdateContextualBasicAttackResets()
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->CharacterInputState.WantsToSprint)
	{
		bSprintContextualBasicAttackConsumed = false;
		bWasSprintAttackContextActive = false;
		return;
	}

	const bool bSprintAttackContextActive = IsSprintAttackContext();
	if (bSprintAttackContextActive && !bWasSprintAttackContextActive)
	{
		bSprintContextualBasicAttackConsumed = false;
	}

	bWasSprintAttackContextActive = bSprintAttackContextActive;
}

int32 UMVCombatComponent::GetDodgeAttackContextInstanceId() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!ActionComponent || !ActionComponent->IsActionRunning())
	{
		return INDEX_NONE;
	}

	const FName ActiveActionTableName = ActionComponent->GetActiveActionTableName();
	const FName ActiveActionRowName = ActionComponent->GetActiveActionRowName();
	const FString ActiveActionTableNameString = ActiveActionTableName.ToString();
	const FString ActiveActionRowNameString = ActiveActionRowName.ToString();
	if (MVCombatIsDodgeContextualAttackRowName(ActiveActionRowNameString))
	{
		return INDEX_NONE;
	}

	const bool bIsDodgeActionContext = ActiveActionTableNameString.StartsWith(TEXT("Dodge"))
		|| ActiveActionRowNameString.StartsWith(TEXT("Dodge_"));
	return bIsDodgeActionContext ? ActionComponent->GetActiveActionInstanceId() : INDEX_NONE;
}

bool UMVCombatComponent::IsDodgeAttackContext() const
{
	return GetDodgeAttackContextInstanceId() != INDEX_NONE;
}

bool UMVCombatComponent::IsSprintAttackContext() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	const UMVActionComponent* ActionComponent = OwnerCharacter ? OwnerCharacter->ActionComponent : nullptr;
	if (!OwnerCharacter || (ActionComponent && ActionComponent->IsActionRunning()))
	{
		return false;
	}

	return OwnerCharacter->Gait == EGait::Sprinting
		&& HasReachedSprintAttackSpeed();
}

bool UMVCombatComponent::ShouldSuppressChargeAttackInputForSprint() const
{
	const AMVCharacterBase* OwnerCharacter = Cast<AMVCharacterBase>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->CharacterInputState.WantsToSprint)
	{
		return false;
	}

	return OwnerCharacter->Gait == EGait::Sprinting
		|| bSprintContextualBasicAttackConsumed;
}

bool UMVCombatComponent::HasReachedSprintAttackSpeed() const
{
	if (!StatComponent)
	{
		return false;
	}

	return StatComponent->HasReachedSprintSpeedRatio(SprintAttackMinSpeedRatio);
}
