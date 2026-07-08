// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Public/Struct/MVCombatActionTableInput.h"
#include "Public/Enum/MVCombatActionTypes.h"
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
	bool bIsChained = false;

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
	TObjectPtr<UDataTable> DataTable = nullptr;

	// Main cooldown duration (applies to entire skill or chain)
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float MainCooldownDuration = 0.0f;

	// Last time this skill was used (for cooldown tracking)
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float LastUsedTime = 0.0f;

	// For chained skills: current stage index
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	int32 CurrentChainStageIndex = 0;

	// For chained skills: whether chain is currently active
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bChainActive = false;

	// For chained skills: time when current stage started
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float LastStageActivationTime = 0.0f;

	// For chained skills: when input window closes for current stage
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float InputWindowCloseTime = 0.0f;

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
		if (!DataTable || !SkillRowNames.IsValidIndex(CurrentChainStageIndex))
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

	bool ContainsAbility(const UMVAbilityBase* Ability) const
	{
		if (!Ability)
		{
			return false;
		}

		for (const TObjectPtr<UMVAbilityBase>& AbilityInstance : AbilityInstances)
		{
			if (AbilityInstance.Get() == Ability)
			{
				return true;
			}
		}

		return false;
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

	FDataTableRowHandle GetCurrentActionRowHandle() const
	{
		FDataTableRowHandle RowHandle;
		RowHandle.DataTable = DataTable.Get();
		RowHandle.RowName = GetCurrentRowName();
		return RowHandle;
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

	void StartPostAbilityResetWindow(float CurrentTime)
	{
		if (bChainActive)
		{
			LastStageActivationTime = CurrentTime;

			const FMVSkillDataTableColumn* CurrentData = GetCurrentSkillData();
			if (CurrentData)
			{
				InputWindowCloseTime = CurrentTime + CurrentData->InputWindowDuration;
			}
			return;
		}

		LastUsedTime = CurrentTime;
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
Fetch action RowHandle from ChooserTable and run chained action rows.
Sprint/Dodge contextual basic attacks are consumed once per source context so single-row contextual attacks do not replay from held or repeated input.

Note:	Getting action RowHandle through ChooserTable should be implemented in blueprint.
		DataTable-only lookup remains as a compatibility fallback.

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
	bool TryCombatAction(EMVCombatActionTypes InActionType, int32 ActionIndex = 0, FName StartSection = NAME_None);

	void HandleAbilityEnded(const UMVAbilityBase* EndedAbility);

	// Call When Character Change Weapon --> have to receive Event from Character
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void ChangeWeapon(EMVEquippedStyle NewStyle);

protected:
	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;
	bool ChooseTryCombatAction(FGameplayTag ActionInputTag);
	bool IsCombatActionInputTag(FGameplayTag ActionInputTag) const;

	bool TryBasicAttack(EMVCombatActionTypes InActionType, int32 ActionIndex = 0, FName StartSection = NAME_None);
	bool TrySkill(EMVCombatActionTypes InActionType, int32 SkillIndex = 0, FName StartSection = NAME_None);

	// Call when Beginplay or Change Weapon Style
	void RefreshActionMaps();
	
	void ResetBasicAttackMap();
	void ResetSkillMap();
	
protected:
	// Legacy fallback: should return DataTable using ChooserTable in Blueprint.
	// Because Using ChooserTable in C++ is So Fucking Shit
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable,Category = "Data")
	UDataTable* GetDataTableFromChooserTable(const FMVCombatActionTableInput& ChooserInput, bool& OutResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Data")
	FMVSkillDataTableColumn GetDataTableRowFromChooserTable(const FMVCombatActionTableInput& ChooserInput, const FName& RowName, bool& OutResult);
	virtual FMVSkillDataTableColumn GetDataTableRowFromChooserTable_Implementation(const FMVCombatActionTableInput& ChooserInput, const FName& RowName, bool& OutResult);

private:
	bool BuildSkillEntryFromRowHandle(const FDataTableRowHandle& StartingRowHandle, FMVSkillEntry& OutEntry);
	bool GetActionRowHandleFromChooserTable(
		const FMVCombatActionTableInput& ChooserInput,
		FName FallbackRowName,
		FDataTableRowHandle& OutRowHandle) const;
	bool ResolveActionRowHandleFromChooserTable(
		const FMVCombatActionTableInput& ChooserInput,
		FName FallbackRowName,
		FDataTableRowHandle& OutRowHandle);
	UDataTable* LoadFallbackAttackActionTable() const;
	bool TryMakeFallbackAttackActionRowHandle(
		EMVCombatActionTypes ActionType,
		FDataTableRowHandle& OutRowHandle) const;
	bool IsValidSkillActionRowHandle(const FDataTableRowHandle& RowHandle, const TCHAR* Context) const;
	bool IsCurrentAbilityAction(FName ActionTableName, FName ActionRowName) const;
	FName MakeActionTypeMapKey(EMVCombatActionTypes ActionType) const;
	FName MakeIndexedActionRowName(EMVCombatActionTypes ActionType, int32 ActionIndex) const;
	FGameplayTag MakeActionTypeGameplayTag(EMVCombatActionTypes ActionType) const;
	bool TryStartActionWithAbility(FMVSkillEntry& ActionEntry, const FDataTableRowHandle& RowHandle, FName StartSection = NAME_None);
	bool CanConsumeActionCost(const FMVSkillDataTableColumn* SkillData) const;
	bool IsBasicAttackActionType(EMVCombatActionTypes ActionType) const;
	int32 SelectBasicAttackChainStageForSwing(const FMVSkillEntry& ActionEntry) const;
	const FMVSkillDataTableColumn* GetBasicAttackSkillDataAtStage(const FMVSkillEntry& ActionEntry, int32 ChainStageIndex) const;
	void UpdateLastBasicAttackSwingDirection(const FDataTableRowHandle& RowHandle);
	void ClearLastBasicAttackSwingDirection();
	EMVCombatActionTypes ResolveContextualBasicAttackActionType(EMVCombatActionTypes RequestedActionType);
	void MarkContextualBasicAttackStarted(EMVCombatActionTypes StartedActionType);
	void UpdateContextualBasicAttackResets();
	int32 GetDodgeAttackContextInstanceId() const;
	bool IsDodgeAttackContext() const;
	bool IsSprintAttackContext() const;
	bool ShouldSuppressChargeAttackInputForSprint() const;
	bool HasReachedSprintAttackSpeed() const;

	UFUNCTION()
	void HandleActionEnded(FName ActionTableName, FName ActionRowName, bool bInterrupted);

public:

	// LightAttack, HeavyAttack, ChargeAttack, and contextual sprint/dodge attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<FName, FMVSkillEntry>BasicAttackMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<FName, FMVSkillEntry>SkillMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	EMVEquippedStyle CurrentWeaponStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	float ResetBasicAttackTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Sprint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SprintAttackMinSpeedRatio = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Chooser")
	FSoftObjectPath AttackChooserTable = TEXT("/Game/Table/Weapons/ActionTables/CHT_PlayerAttack.CHT_PlayerAttack");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Chooser")
	FSoftObjectPath FallbackAttackActionTable = TEXT("/Game/Table/Weapons/ActionTables/Yone_Attack.Yone_Attack");

	UPROPERTY(Transient)
	TObjectPtr<UMVAbilityBase> PreviousAbilityInstance;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVAbilityBase> CurrentAbilityInstance;
	FName CurrentAbilityActionTableName = NAME_None;
	FName CurrentAbilityActionRowName = NAME_None;

private:
	double LastBasicAttackedTime = 0.0;
	FName ActiveBasicAttackMapKey = NAME_None;
	TObjectPtr<UMVStatComponent> StatComponent;
	bool bSprintContextualBasicAttackConsumed = false;
	bool bWasSprintAttackContextActive = false;
	int32 ConsumedDodgeContextActionInstanceId = INDEX_NONE;
	int32 PendingDodgeContextActionInstanceId = INDEX_NONE;
	EMVAttackSwingDirection LastBasicAttackSwingDirection = EMVAttackSwingDirection::None;

};
