// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVCombatComponent.h"

#include "Chooser.h"
#include "ChooserFunctionLibrary.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVActionComponent.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVStatComponent.h"
#include "Public/Interface/MVAbilityInterface.h"
#include "Tags/MVGameplayTags.h"

// Sets default values for this component's properties
UMVCombatComponent::UMVCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UMVCombatComponent::BeginPlay()
{
	CurrentWeaponStyle = EMVEquippedStyle::BareHand; // Test


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
	}

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UMVCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...
}

bool UMVCombatComponent::TryCombatAction(EMVCombatActionTypes InActionType, int32 SkillIndex)
{
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
		if (InActionType == EMVCombatActionTypes::LightAttack ||
			InActionType == EMVCombatActionTypes::HeavyAttack ||
			InActionType == EMVCombatActionTypes::ChargeAttack)
		{
			return TryBasicAttack(InActionType);
		}
		else if (InActionType == EMVCombatActionTypes::Skill)
		{
			return TrySkill(InActionType, SkillIndex);
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

bool UMVCombatComponent::ChooseTryCombatAction(const FGameplayTag ActionInputTag)
{
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
	else if (ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_ChargeAttack))
	{
		return TryCombatAction(EMVCombatActionTypes::ChargeAttack);
	}

	return false;
}

bool UMVCombatComponent::IsCombatActionInputTag(const FGameplayTag ActionInputTag) const
{
	return ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_LightAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_HeavyAttack)
		|| ActionInputTag.MatchesTagExact(MVGameplayTags::Action_Input_ChargeAttack)
		|| ActionInputTag.MatchesTag(MVGameplayTags::Action_Input_Skill);
}




bool UMVCombatComponent::TryBasicAttack(EMVCombatActionTypes InActionType)
{
	FName RowName;
	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
	FName TypeName = FName(*EnumPtr->GetNameStringByValue((int64)InActionType));

	RowName = TypeName;
	FMVSkillEntry* ActionEntry = BasicAttackMap.Find(RowName);

	if (!ActionEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Action '%s' not found"), *RowName.ToString());
		return false;
	}

	if(!CanConsumeActionCost(ActionEntry->GetCurrentSkillData()))
	{
		return false;
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

		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
			PreviousAbilityInstance = CurrentAbilityInstance;
			if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
			{
				IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
			}
			CurrentAbilityInstance = ActionEntry->GetCurrentAbility();
			ActionEntry->ActivateChain(CurrentTime);
			ActionEntry->TryAdvanceChainStage(CurrentTime);
			LastBasicAttackedTime = CurrentTime;
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Failed to send data to ActionComponent for action '%s'"), *RowName.ToString());
			return false;
		}

	}

	if (ActionEntry && ActionEntry->bChainActive)
	{
		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
			PreviousAbilityInstance = CurrentAbilityInstance;
			if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
			{
				IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
			}
			CurrentAbilityInstance = ActionEntry->GetCurrentAbility();
			ActionEntry->TryAdvanceChainStage(CurrentTime);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Failed to send data to ActionComponent for action '%s'"), *RowName.ToString());
			return false;
		}
		
	}
	else
	{
		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
			PreviousAbilityInstance = CurrentAbilityInstance;
			if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
			{
				IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
			}
			CurrentAbilityInstance = ActionEntry->GetCurrentAbility();
			ActionEntry->ActivateChain(CurrentTime);
			ActionEntry->TryAdvanceChainStage(CurrentTime);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::TryBasicAttack - Failed to send data to ActionComponent for action '%s'"), *RowName.ToString());
			return false;
		}
	}
		LastBasicAttackedTime = CurrentTime;
		return true;

}

bool UMVCombatComponent::TrySkill(EMVCombatActionTypes InActionType, int32 SkillIndex)
{
	FName RowName;

	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
	FName TypeName = FName(*EnumPtr->GetNameStringByValue((int64)InActionType));

	FString TypeString = TypeName.ToString() + FString::FromInt(SkillIndex);
	RowName = FName(*TypeString);

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

			if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
			{
				PreviousAbilityInstance = CurrentAbilityInstance;
				if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
				{
					IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
				}
				CurrentAbilityInstance = ActionEntry->GetCurrentAbility();
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

			if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
			{
				PreviousAbilityInstance = CurrentAbilityInstance;
				if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
				{
					IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
				}
				CurrentAbilityInstance = ActionEntry->GetCurrentAbility();
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

		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
			PreviousAbilityInstance = CurrentAbilityInstance;
			if (PreviousAbilityInstance && PreviousAbilityInstance->Implements<UMVAbilityInterface>())
			{
				IMVAbilityInterface::Execute_EndAbility(PreviousAbilityInstance);
			}
			CurrentAbilityInstance = ActionEntry->GetCurrentAbility();
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
	
	bool DataTableSearchResult = false;
	FMVCombatActionTableInput ChooserInput;
	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;

	TArray<EMVCombatActionTypes> TypeArray;
	TypeArray.Add(EMVCombatActionTypes::LightAttack);
	TypeArray.Add(EMVCombatActionTypes::HeavyAttack);
	TypeArray.Add(EMVCombatActionTypes::ChargeAttack);

	for (EMVCombatActionTypes Types : TypeArray)
	{
		ChooserInput.ActionType = Types;

		// Find DataTable using ChooserTable input
		UDataTable* CurrentDT = GetDataTableFromChooserTable(ChooserInput, DataTableSearchResult);

		if (!DataTableSearchResult)
		{
			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map): Cannot Get DataTable"));
			return;
		}
		
		TSet<FName> ProcessedRowNames;

		for (auto& It : CurrentDT->GetRowMap())
		{
			FMVSkillDataTableColumn* RowData = reinterpret_cast<FMVSkillDataTableColumn*>(It.Value);
			if (!RowData || !RowData->AbilityReference)
			{
				UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent(Reset Skill Map): Invalid row data or ability reference for '%s'"), *It.Key.ToString());
				continue;
			}

			// Chained Skill will add previous when first skill(first Chain skill) add to the map
			if (ProcessedRowNames.Contains(It.Key))
			{
				continue;
			}

			FMVSkillEntry SkillEntry;
			SkillEntry.DataTable = CurrentDT;
			SkillEntry.bIsChained = RowData->bIsChained;
			SkillEntry.MainCooldownDuration = RowData->CooldownDuration;
			SkillEntry.LastUsedTime = 0.0f;
			SkillEntry.CurrentChainStageIndex = 0;
			SkillEntry.bChainActive = false;

			if (RowData->bIsChained && !RowData->NextChainName.IsNone())
			{
				// Build Chained Skill and collect all processed row names
				BuildChainedEntry(It.Key, *CurrentDT, SkillEntry);

				// Add all row names from this chain to the processed set
				for (const FName& RowName : SkillEntry.SkillRowNames)
				{
					ProcessedRowNames.Add(RowName);
				}
			}
			else
			{
				// Simple Skill - One Ability
				UMVAbilityBase* AbilityInstance = NewObject<UMVAbilityBase>(this, RowData->AbilityReference);
				if (AbilityInstance)
				{
					AbilityInstance->SetOwner(this);
					AbilityInstance->InitAbility(*RowData);
					SkillEntry.AbilityInstances.Add(AbilityInstance);
					SkillEntry.SkillRowNames.Add(It.Key);
				}
				ProcessedRowNames.Add(It.Key);
			}

			BasicAttackMap.Add(It.Key, SkillEntry);
		}
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
	ChooserInput.ActionType = EMVCombatActionTypes::Skill;

	// Find DataTable using ChooserTable input
	UDataTable* CurrentDT = GetDataTableFromChooserTable(ChooserInput, DataTableSearchResult);

	if (!DataTableSearchResult)
	{
		UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map): Cannot Get DataTable"));
		return;
	}

	TSet<FName> ProcessedRowNames;

	for (auto& It : CurrentDT->GetRowMap())
	{
		FMVSkillDataTableColumn* RowData = reinterpret_cast<FMVSkillDataTableColumn*>(It.Value);

		if (!RowData || !RowData->AbilityReference)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent(Reset Skill Map): Invalid row data or ability reference for '%s'"), *It.Key.ToString());
			continue;
		}

		// Chained Skill will add previous when first skill(first Chain skill) add to the map
		//if (SkillMap.Contains(It.Key))
		//{
		//	continue;
		//}
		if (ProcessedRowNames.Contains(It.Key))
		{
			continue;
		}

		FMVSkillEntry SkillEntry;
		SkillEntry.DataTable = CurrentDT;
		SkillEntry.bIsChained = RowData->bIsChained;
		SkillEntry.MainCooldownDuration = RowData->CooldownDuration;
		SkillEntry.LastUsedTime = 0.0f;
		SkillEntry.CurrentChainStageIndex = 0;
		SkillEntry.bChainActive = false;

		if (RowData->bIsChained && !RowData->NextChainName.IsNone())
		{
			// Build Chained Skill
			BuildChainedEntry(It.Key, *CurrentDT, SkillEntry);

			// Add all row Names From this Chain to ProcessedRowNames to avoid duplicates
			for(const FName& RowName : SkillEntry.SkillRowNames)
			{
				ProcessedRowNames.Add(RowName);
			}
		}
		else
		{
			// Simple Skill - One Chain Skill
			UMVAbilityBase* AbilityInstance = NewObject<UMVAbilityBase>(this, RowData->AbilityReference);
			if (AbilityInstance)
			{
				AbilityInstance->SetOwner(this);
				AbilityInstance->InitAbility(*RowData);
				SkillEntry.AbilityInstances.Add(AbilityInstance);
				SkillEntry.SkillRowNames.Add(It.Key);
			}
			ProcessedRowNames.Add(It.Key);
		}
		SkillMap.Add(It.Key, SkillEntry);
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

	// Get DataTable along with ChooserInput
	bool DataTableResult{};
	UDataTable* OutDataTable = GetDataTableFromChooserTable(ChooserInput, DataTableResult);
	if (!OutDataTable)
	{
		// When DataTable is not valid
		OutResult = false;
		return OutRow;
	}

	// Find Row
	FMVSkillDataTableColumn* Data = OutDataTable->FindRow<FMVSkillDataTableColumn>(RowName, TEXT("DataTableRow"));
	
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

void UMVCombatComponent::BuildChainedEntry(const FName& StartingName, const UDataTable& CurrentDT, FMVSkillEntry& OutEntry)
{
	FName CurrentName = StartingName;
	int32 ChainLength = 0;
	int32 MaxChainLength = 100;

	while (!CurrentName.IsNone() && ChainLength < MaxChainLength)
	{
		FMVSkillDataTableColumn* RowData = CurrentDT.FindRow<FMVSkillDataTableColumn>(CurrentName, TEXT("BuildChainedSkillEntry"));
		if (!RowData || !RowData->AbilityReference)
		{
			UE_LOG(LogTemp, Warning, TEXT("MVCombatComponent::BuildChainedSkillEntry - Invalid row '%s'"), *CurrentName.ToString());
			break;
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
	OutEntry.bIsChained = OutEntry.AbilityInstances.Num() > 1;
}

bool UMVCombatComponent::SendDataToActionComp(EMVCombatActionTypes InActionType, FName RowName)
{
	FMVCombatActionTableInput ChooserInput;
	ChooserInput.ActionType = InActionType;
	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;
	bool Result;

	FDataTableRowHandle RowHandle;
	RowHandle.DataTable = GetDataTableFromChooserTable(ChooserInput, Result);
	
	if (!RowHandle.DataTable)
	{
		return false;
	}

	RowHandle.RowName = RowName;

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

	// If an action is running, only allow transition when recovery is open or action is interruptible.
	if (bIsActionRunning)
	{
		if (bRecoveryOpen && bCanInterrupt)
		{
			Result = ActionComponent->TryTransitionActionFromRowHandle(RowHandle);
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
		Result = ActionComponent->TryStartActionFromRowHandle(RowHandle);
	}

	return Result;
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

	if (!StatComponent->HasStamina(SkillData->StaminaCost))
	{
		return false;
	}
	
	if (!StatComponent->HasMP(SkillData->MpCost))
	{
		return false;
	}

	return true;
}

