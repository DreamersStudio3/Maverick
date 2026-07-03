// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVCombatComponent.h"

#include "Chooser.h"
#include "ChooserFunctionLibrary.h"
#include "Combat/MVAbilityBase.h"
#include "Components/MVActionComponent.h"
#include "Character/MVCharacterBase.h"

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
	
}


// Called every frame
void UMVCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...
}

bool UMVCombatComponent::TryCombatAction(EMVCombatActionTypes InActionType, int32 SkillIndex)
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
	// Todo: ChargeAttack --> Pressed will be same as basic attack, But key Release of ChargeSkill, Another function need
	return false;
}

bool UMVCombatComponent::TryBasicAttack(EMVCombatActionTypes InActionType)
{
	FName RowName;
	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
	FName TypeName = FName(*EnumPtr->GetNameStringByValue((int64)InActionType));

	RowName = TypeName;
	FMVSkillEntry* ActionEntry = BasicAttackMap.Find(RowName);

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Reset Basic Attack's Index and Start Attack
	if (CurrentTime - LastBasicAttackedTime > ResetBasicAttackTime)
	{
		for (TPair<FName, FMVSkillEntry>& pair : BasicAttackMap)
		{
			FMVSkillEntry& CurrentEntry = pair.Value;
			CurrentEntry.ResetChain();
		}

		// Todo: Should Check Stat Component

		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
			ActionEntry->ActivateChain(CurrentTime);
			LastBasicAttackedTime = CurrentTime;
			ActionEntry->TryAdvanceChainStage(CurrentTime);
			return true;
		}
		else
		{
			return false;
		}

	}

	if (ActionEntry && ActionEntry->bChainActive)
	{
		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
			ActionEntry->TryAdvanceChainStage(CurrentTime);
		}
		
	}
	else
	{
		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
			ActionEntry->ActivateChain(CurrentTime);
			ActionEntry->TryAdvanceChainStage(CurrentTime);
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

			// Todo: Should Check Stat Component

			if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
			{
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
			// Todo: Should Check Stat Component

			if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
			{
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
		
		// Todo: Should Check Stat Component

		if (SendDataToActionComp(InActionType, ActionEntry->GetCurrentRowName()))
		{
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

//bool UMVCombatComponent::TryBasicAttack(EMVCombatActionTypes InActionType, FMVSkillDataTableColumn& OutRow)
//{
//	double AttackInTime = GetWorld()->GetTimeSeconds();
//	// If attack time elapsed specific time, reset the basic attack index
//	if (AttackInTime - LastBasicAttackedTime > ResetBasicAttackIndexTime)
//	{
//		//ResetCurrentIndex();
//	}
//
//	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
//	FName TypeName = FName(*EnumPtr->GetNameStringByValue((int64)InActionType));
//
//	int32* CurrentIndex = BasicAttackCurrentIndex.Find(TypeName);
//	int32* MaxIndex = BasicAttackMaxIndex.Find(TypeName);
//
//	if (!CurrentIndex || !MaxIndex || *MaxIndex < 0)
//	{
//		return false;
//	}
//
//	FString RowString = TypeName.ToString() + FString::FromInt(*CurrentIndex);
//	FName TargetRowName = FName(*RowString);
//	
//	FMVCombatActionTableInput ChooserInput;
//	ChooserInput.ActionType = InActionType;
//	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;
//
//	bool TableRowResult;
//
//	OutRow = GetDataTableRowFromChooserTable_Implementation(ChooserInput, TargetRowName, TableRowResult);
//	if (TableRowResult)
//	{
//		LastBasicAttackedTime = AttackInTime;
//		int32 NextIndex = (*CurrentIndex + 1) % (*MaxIndex);
//		BasicAttackCurrentIndex.Add(TypeName, NextIndex);
//
//		UE_LOG(LogTemp, Log, TEXT("%s attack tried"),*RowString);
//
//		return true;
//	}
//
//	return false;
//}

//bool UMVCombatComponent::TrySkill(uint32 SkillIndex, bool FullyStacked)
//{
//	if (SkillIndex >= SkillMaxIndex)
//	{
//		return false;
//	}
//
//	TArray<FMVSkillEntry> ValueArray;
//	SkillMap.GenerateValueArray(ValueArray);
//
//	if (ValueArray.IsValidIndex(SkillIndex))
//	{
//		if (!ValueArray[SkillIndex].AbilityInstance->CheckCooldown())
//		{
//			return false;
//		}
//
//		// Todo: Check Stat Component
//		//return false;
//
//		// Send Animation Data to ActionComponent
//		FDataTableRowHandle RowHandle;
//		RowHandle.DataTable = ValueArray[SkillIndex].DataTable;
//		RowHandle.RowName = ValueArray[SkillIndex].RowName;
//
//		AMVCharacterBase* Owner = Cast<AMVCharacterBase>(GetOwner());
//		if (!Owner)
//		{
//			return false;
//		}
//		//Owner->ActionComponent->TryTransitionActionFromRowHandle(RowHandle);
//		if (Owner->ActionComponent->TryStartActionFromRowHandle(RowHandle))
//		{
//			ValueArray[SkillIndex].AbilityInstance->UpdateLastUsedTime();
//			return true;
//		}
//
//	}
//
//	return false;
//}


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
					SkillEntry.SkillDataArray.Add(*RowData);
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
				SkillEntry.SkillDataArray.Add(*RowData);
			}
			ProcessedRowNames.Add(It.Key);
		}
		SkillMap.Add(It.Key, SkillEntry);
	}

}

//void UMVCombatComponent::ResetBasicAttackMap()
//{
//	// Clear Map
//	if (BasicAttackMaxIndex.Num() != 0 || BasicAttackCurrentIndex.Num() != 0)
//	{
//		BasicAttackMaxIndex.Reset();
//		BasicAttackCurrentIndex.Reset();
//	}
//	// Enum Pointer -> To extract FName of Enum
//	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
//
//	// Variables for Searching DataBase
//	bool DataTableSearchResult = false;
//	FMVCombatActionTableInput ChooserInput;
//	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;
//
//	// Will Make Map only wanting Enums (LightAttack, HeavyAttack)
//	for (EMVCombatActionTypes Type : WantToMapActionTypes)
//	{
//		ChooserInput.ActionType = Type;
//		UDataTable* CurrentDT = GetDataTableFromChooserTable(ChooserInput, DataTableSearchResult);
//		if (DataTableSearchResult == false)
//		{
//			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(BeginPlay): Cannot Get DataTable"));
//			return;
//		}
//		UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(BeginPlay):Get DataTable"));
//
//		// Get DataTable Map Reference
//		const TMap<FName, uint8*>& RowMap = CurrentDT->GetRowMap();
//
//		// Extract FString and FName of Enum
//		FString TypeString = EnumPtr->GetNameStringByValue((int64)Type);
//		FName TypeName = FName(*TypeString);
//
//		int32 Count = 0;
//
//		// Counting DataTable's Row numbers and set MaxIndex
//		for (auto& Iter : RowMap)
//		{
//			// if RowName match with Enum String, Count++
//			if (Iter.Key.ToString().StartsWith(TypeString))
//			{
//				Count++;
//			}
//		}
//
//		// Only Add when Count is valid integer
//		if (Count > 0)
//		{
//			//RowMap.Num() -> If DataTable Contains only One Type Attack
//			BasicAttackMaxIndex.Add(TypeName, Count);
//			BasicAttackCurrentIndex.Add(TypeName, 0);
//		}
//
//	}
//}

//void UMVCombatComponent::ResetSkillMap()
//{
//	if (SkillMap.Num() != 0)
//	{
//		SkillMap.Reset();
//	}
//
//	// Enum Pointer -> To extract FName of Enum
//	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
//
//	// Variables for Searching DataBase
//	bool DataTableSearchResult = false;
//	FMVCombatActionTableInput ChooserInput;
//	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;
//	ChooserInput.ActionType = EMVCombatActionTypes::Skill;
//
//	UDataTable* CurrentDT = GetDataTableFromChooserTable(ChooserInput, DataTableSearchResult);
//
//	if (DataTableSearchResult == false)
//	{
//		UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map): Cannot Get DataTable"));
//		return;
//	}
//	UE_LOG(LogTemp, Log, TEXT("MVCombatComponentReset Skill Map):Get DataTable"));
//
//	for (auto& It : CurrentDT->GetRowMap())
//	{
//		FMVSkillActionStruct SkillData;
//		
//		FMVSkillDataTableColumn* RowData = reinterpret_cast<FMVSkillDataTableColumn*>(It.Value);
//		if (!RowData)
//		{
//			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map):Get RowData"));
//			return;
//		}
//		if (!RowData->AbilityReference)
//		{
//			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map):Nullptr -> AbilityReference"));
//			return;
//		}
//		SkillData.AbilityInstance = NewObject<UMVAbilityBase>(this, RowData->AbilityReference);
//		
//		// Ability should set Owner and InitAbility(DataTable)
//		SkillData.AbilityInstance->SetOwner(this);
//		SkillData.AbilityInstance->InitAbility(*RowData);
//
//		SkillData.RowName = It.Key;
//		SkillData.DataTable = CurrentDT;
//
//		SkillMap.Add(RowData->AbilityReference, SkillData);
//	}
//	SkillMaxIndex = SkillMap.Num();
//
//}

// Set all current indices to 0
//void UMVCombatComponent::ResetCurrentIndex()
//{
//	for (auto& Element : BasicAttackCurrentIndex)
//	{
//		Element.Value = 0;
//	}
//}

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
			OutEntry.SkillDataArray.Add(*RowData);
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
	//if (Owner->ActionComponent->TryStartActionFromRowHandle(RowHandle))
	if (Owner->ActionComponent->TryTransitionActionFromRowHandle(RowHandle))
	{
		return true;
	}
	return false;
}

