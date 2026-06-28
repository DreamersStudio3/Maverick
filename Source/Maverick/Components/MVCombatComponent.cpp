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

	if (WantToMapActionTypes.Num() == 0)
	{
		WantToMapActionTypes.Add(EMVCombatActionTypes::LightAttack);
		WantToMapActionTypes.Add(EMVCombatActionTypes::HeavyAttack);
	}

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

bool UMVCombatComponent::TryCombatAction(EMVCombatActionTypes InActionType, FMVSkillDataTableColumn& OutRow, bool FullyStacked, int32 SkillIndex)
{
	bool isBasicAttack = false;
	for (EMVCombatActionTypes Type : WantToMapActionTypes)
	{
		if (Type == InActionType)
		{
			isBasicAttack = true;
			break;
		}
	}
	// Basic Attack uses Index
	if (isBasicAttack)
	{
		return TryBasicAttack(InActionType, OutRow);
	}

	// Action Type is Skill (Charge attack or Cooltime Skill)
	else if (InActionType == EMVCombatActionTypes::Skill)
	{
		return TrySkill(SkillIndex, FullyStacked);
	}

	// Other Actions --> If Other action should concern, add logic
	// Todo: ChargeAttack --> Pressed will be same as basic attack, But key Release of ChargeSkill, Another function need
	return false;
}

bool UMVCombatComponent::TryBasicAttack(EMVCombatActionTypes InActionType, FMVSkillDataTableColumn& OutRow)
{
	double AttackInTime = GetWorld()->GetTimeSeconds();
	// If attack time elapsed specific time, reset the basic attack index
	if (AttackInTime - LastBasicAttackedTime > ResetBasicAttackIndexTime)
	{
		ResetCurrentIndex();
	}

	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();
	FName TypeName = FName(*EnumPtr->GetNameStringByValue((int64)InActionType));

	int32* CurrentIndex = BasicAttackCurrentIndex.Find(TypeName);
	int32* MaxIndex = BasicAttackMaxIndex.Find(TypeName);

	if (!CurrentIndex || !MaxIndex || *MaxIndex < 0)
	{
		return false;
	}

	FString RowString = TypeName.ToString() + FString::FromInt(*CurrentIndex);
	FName TargetRowName = FName(*RowString);
	
	FMVCombatActionTableInput ChooserInput;
	ChooserInput.ActionType = InActionType;
	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;

	bool TableRowResult;

	OutRow = GetDataTableRowFromChooserTable_Implementation(ChooserInput, TargetRowName, TableRowResult);
	if (TableRowResult)
	{
		LastBasicAttackedTime = AttackInTime;
		int32 NextIndex = (*CurrentIndex + 1) % (*MaxIndex);
		BasicAttackCurrentIndex.Add(TypeName, NextIndex);

		UE_LOG(LogTemp, Log, TEXT("%s attack tried"),*RowString);

		return true;
	}

	return false;
}

bool UMVCombatComponent::TrySkill(uint32 SkillIndex, bool FullyStacked)
{
	if (SkillIndex >= SkillMaxIndex)
	{
		return false;
	}

	TArray<FMVSkillActionStruct> ValueArray;
	SkillMap.GenerateValueArray(ValueArray);

	if (ValueArray.IsValidIndex(SkillIndex))
	{
		if (!ValueArray[SkillIndex].AbilityInstance->CheckCooldown())
		{
			return false;
		}

		// Todo: Check Stat Component
		//return false;

		// Todo: Send Animation Data to ActionComponent
		FDataTableRowHandle RowHandle;
		RowHandle.DataTable = ValueArray[SkillIndex].DataTable;
		RowHandle.RowName = ValueArray[SkillIndex].RowName;

		AMVCharacterBase* Owner = Cast<AMVCharacterBase>(GetOwner());
		if (!Owner)
		{
			return false;
		}
		Owner->ActionComponent->TryTransitionActionFromRowHandle(RowHandle);
		return true;

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
	// Clear Map
	if (BasicAttackMaxIndex.Num() != 0 || BasicAttackCurrentIndex.Num() != 0)
	{
		BasicAttackMaxIndex.Reset();
		BasicAttackCurrentIndex.Reset();
	}
	// Enum Pointer -> To extract FName of Enum
	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();

	// Variables for Searching DataBase
	bool DataTableSearchResult = false;
	FMVCombatActionTableInput ChooserInput;
	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;

	// Will Make Map only wanting Enums (LightAttack, HeavyAttack)
	for (EMVCombatActionTypes Type : WantToMapActionTypes)
	{
		ChooserInput.ActionType = Type;
		UDataTable* CurrentDT = GetDataTableFromChooserTable(ChooserInput, DataTableSearchResult);
		if (DataTableSearchResult == false)
		{
			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(BeginPlay): Cannot Get DataTable"));
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(BeginPlay):Get DataTable"));

		// Get DataTable Map Reference
		const TMap<FName, uint8*>& RowMap = CurrentDT->GetRowMap();

		// Extract FString and FName of Enum
		FString TypeString = EnumPtr->GetNameStringByValue((int64)Type);
		FName TypeName = FName(*TypeString);

		int32 Count = 0;

		// Counting DataTable's Row numbers and set MaxIndex
		for (auto& Iter : RowMap)
		{
			// if RowName match with Enum String, Count++
			if (Iter.Key.ToString().StartsWith(TypeString))
			{
				Count++;
			}
		}

		// Only Add when Count is valid integer
		if (Count > 0)
		{
			//RowMap.Num() -> If DataTable Contains only One Type Attack
			BasicAttackMaxIndex.Add(TypeName, Count);
			BasicAttackCurrentIndex.Add(TypeName, 0);
		}

	}
}

void UMVCombatComponent::ResetSkillMap()
{
	if (SkillMap.Num() != 0)
	{
		SkillMap.Reset();
	}

	// Enum Pointer -> To extract FName of Enum
	const UEnum* EnumPtr = StaticEnum<EMVCombatActionTypes>();

	// Variables for Searching DataBase
	bool DataTableSearchResult = false;
	FMVCombatActionTableInput ChooserInput;
	ChooserInput.CurrentWeaponStyle = CurrentWeaponStyle;
	ChooserInput.ActionType = EMVCombatActionTypes::Skill;

	UDataTable* CurrentDT = GetDataTableFromChooserTable(ChooserInput, DataTableSearchResult);

	if (DataTableSearchResult == false)
	{
		UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map): Cannot Get DataTable"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("MVCombatComponentReset Skill Map):Get DataTable"));

	for (auto& It : CurrentDT->GetRowMap())
	{
		FMVSkillActionStruct SkillData;
		
		FMVSkillDataTableColumn* RowData = reinterpret_cast<FMVSkillDataTableColumn*>(It.Value);
		if (!RowData)
		{
			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map):Get RowData"));
			return;
		}
		if (!RowData->AbilityReference)
		{
			UE_LOG(LogTemp, Log, TEXT("MVCombatComponent(Reset Skill Map):Nullptr -> AbilityReference"));
			return;
		}
		SkillData.AbilityInstance = NewObject<UMVAbilityBase>(this, RowData->AbilityReference);
		SkillData.AbilityInstance->SetOwner(this);
		SkillData.AbilityInstance->InitAbility(*RowData);

		SkillData.RowName = It.Key;
		SkillData.DataTable = CurrentDT;

		SkillMap.Add(RowData->AbilityReference, SkillData);
	}
	SkillMaxIndex = SkillMap.Num();

}

// Set all current indices to 0
void UMVCombatComponent::ResetCurrentIndex()
{
	for (auto& Element : BasicAttackCurrentIndex)
	{
		Element.Value = 0;
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

