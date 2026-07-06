// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Public/Struct/MVCombatActionTableInput.h"
#include "Components/MVInputManagerComponent.h"
#include "Interface/MVActionInputHandlerInterface.h"

#include "MVCombatComponent.generated.h"

class UMVAbilityBase;


USTRUCT(BlueprintType)
struct FMVSkillActionStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMVAbilityBase> AbilityInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> DataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;

};

USTRUCT(BlueprintType)
struct FMVSkillEntry
{
	GENERATED_BODY()

public:
	// Indicates if this is a chained skill
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bIsChained;

	// For simple skills: single ability instance
	// For chained skills: multiple ability instances (one per stage)
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TArray<TObjectPtr<UMVAbilityBase>> AbilityInstances;

	// For simple skills: single row name
	// For chained skills: multiple row names in progression order
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TArray<FName> SkillRowNames;

	// DataTable reference
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UDataTable> DataTable;

	// Main cooldown duration (applies to entire skill or chain)
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float MainCooldownDuration;

	// Last time this skill was used (for cooldown tracking)
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float LastUsedTime;

	// For chained skills: current stage index
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 CurrentChainStageIndex;

	// For chained skills: whether chain is currently active
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bChainActive;

	// For chained skills: time when current stage started
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float LastStageActivationTime;

	// For chained skills: when input window closes for current stage
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float InputWindowCloseTime;

public:
	// Get current ability (for simple skills or current chain stage)
	UMVAbilityBase* GetCurrentAbility() const
	{
		if (AbilityInstances.IsValidIndex(CurrentChainStageIndex))
		{
			return AbilityInstances[CurrentChainStageIndex];
		}
		return nullptr;
	}

	// Get current skill data (for simple skills or current chain stage)
	const FMVSkillDataTableColumn* GetCurrentSkillData() const
	{
		if (!DataTable)
		{
			return nullptr;
		}
		const FMVSkillDataTableColumn* SkillData = DataTable->FindRow<FMVSkillDataTableColumn>(SkillRowNames[CurrentChainStageIndex], TEXT(""));
		if (SkillData)
		{
			return SkillData;
		}
		return nullptr;
	}

	// Get current row name (for simple skills or current chain stage)
	FName GetCurrentRowName() const
	{
		if (SkillRowNames.IsValidIndex(CurrentChainStageIndex))
		{
			return SkillRowNames[CurrentChainStageIndex];
		}
		return NAME_None;
	}

	// Check if main cooldown is ready
	bool IsMainCooldownReady(float CurrentTime) const
	{
		if (MainCooldownDuration <= 0.0f)
		{
			return true;
		}
		return LastUsedTime == 0 || (CurrentTime - LastUsedTime) >= MainCooldownDuration;
	}

	// Check if input window is valid for chain advancement
	bool IsInputWindowValid(float CurrentTime) const
	{
		if (!bChainActive)
		{
			return false;
		}
		return CurrentTime < InputWindowCloseTime;
	}

	// Get remaining time in input window
	float GetRemainingInputWindowTime(float CurrentTime) const
	{
		if (!bChainActive)
		{
			return 0.0f;
		}
		float Remaining = InputWindowCloseTime - CurrentTime;
		return FMath::Max(0.0f, Remaining);
	}

	// Check if inter-stage cooldown is valid for chain advancement
	bool IsInterStageCooldownValid(float CurrentTime) const
	{
		if (!bChainActive || !SkillRowNames.IsValidIndex(CurrentChainStageIndex))
		{
			return false;
		}
		const FMVSkillDataTableColumn* CurrentData = GetCurrentSkillData();
		if (!CurrentData)
		{
			return false;
		}
		return (CurrentTime - LastStageActivationTime) >= CurrentData->InterStageCooldown;
	}

	// Reset chain to stage 0
	void ResetChain()
	{
		CurrentChainStageIndex = 0;
		bChainActive = false;
		LastStageActivationTime = 0.0f;
		InputWindowCloseTime = 0.0f;
	}

	// Activate chain (start from stage 0)
	void ActivateChain(float CurrentTime)
	{
		// If Chain exist(2+ Chained Skills)
		if (bIsChained && AbilityInstances.Num() > 1)
		{
			CurrentChainStageIndex = 0;
			bChainActive = true;
			LastStageActivationTime = CurrentTime;

			const FMVSkillDataTableColumn* CurrentData = GetCurrentSkillData();
			if (CurrentData)
			{
				InputWindowCloseTime = CurrentTime + CurrentData->InputWindowDuration;
			}
		}
		// if Simple Skill or invalid Skill
		else
		{
			bChainActive = false;
			LastUsedTime = CurrentTime;
			return;
		}
	}

	// Advance to next stage in chain
	bool TryAdvanceChainStage(float CurrentTime)
	{
		if (!bChainActive || !bIsChained)
		{
			return false;
		}

		// Check if at last stage
		if (CurrentChainStageIndex >= AbilityInstances.Num() - 1)
		{
			bChainActive = false;
			LastUsedTime = CurrentTime;
			CurrentChainStageIndex = 0;
			return false; // Chain complete
		}

		CurrentChainStageIndex++;
		LastStageActivationTime = CurrentTime;

		const FMVSkillDataTableColumn* CurrentData = GetCurrentSkillData();
		if (CurrentData)
		{
			InputWindowCloseTime = CurrentTime + CurrentData->InputWindowDuration;
		}

		return true;
	}
};

/*
CombatComponent
Fetch Data from DataTable through ChooserTable.

Note:	Getting DataTable through ChooserTable should be implemented in blueprint.
		Getting Row Data comes from Getting DataTable through ChooserTable

		DataTable's Row struct is from "Public/Tables/MVSkillDataTableColumn.h"
		if want to modify Struct, Go to the pulic/Tables/MVSkillDataTableColumn.h


*/

class UChooserTable;
class UMVStatComponent;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAVERICK_API UMVCombatComponent : public UActorComponent, public IMVActionInputHandlerInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMVCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Public API
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool TryCombatAction(EMVCombatActionTypes InActionType, int32 SkillIndex = 0);

	// Call When Character Change Weapon --> have to receive Event from Character
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void ChangeWeapon(EMVEquippedStyle NewStyle);

protected:
	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;
	bool ChooseTryCombatAction(FGameplayTag ActionInputTag);
	bool IsCombatActionInputTag(FGameplayTag ActionInputTag) const;

	bool TryBasicAttack(EMVCombatActionTypes InActionType);
	bool TrySkill(EMVCombatActionTypes InActionType, int32 SkillIndex = 0);

	// Call when Beginplay or Change Weapon Style
	void RefreshActionMaps();
	
	void ResetBasicAttackMap();
	void ResetSkillMap();
	
protected:
	// Should Return DataTable using ChooserTable In Blueprint
	// Because Using ChooserTable in C++ is So Fucking Shit
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable,Category = "Data")
	UDataTable* GetDataTableFromChooserTable(const FMVCombatActionTableInput& ChooserInput, bool& OutResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Data")
	FMVSkillDataTableColumn GetDataTableRowFromChooserTable(const FMVCombatActionTableInput& ChooserInput, const FName& RowName, bool& OutResult);
	virtual FMVSkillDataTableColumn GetDataTableRowFromChooserTable_Implementation(const FMVCombatActionTableInput& ChooserInput, const FName& RowName, bool& OutResult);

private:
	void BuildChainedEntry(const FName& StartingName, const UDataTable& CurrentDT, FMVSkillEntry& OutEntry);
	bool SendDataToActionComp(EMVCombatActionTypes InActionType, FName RowName);
	bool CanConsumeActionCost(const FMVSkillDataTableColumn* SkillData) const;
	void ConsumeActionCost(const FMVSkillDataTableColumn* SkillData) const;

public:

	// LightAttack, HeavyAttack, ChargeAttack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<FName, FMVSkillEntry>BasicAttackMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<FName, FMVSkillEntry>SkillMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	EMVEquippedStyle CurrentWeaponStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	float ResetBasicAttackTime = 2.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMVAbilityBase> PreviousAbilityInstance;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVAbilityBase> CurrentAbilityInstance;

private:
	double LastBasicAttackedTime;
	TObjectPtr<UMVStatComponent> StatComponent;

};
