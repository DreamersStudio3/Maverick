// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/MVStatComponent.h"
#include "Components/ActorComponent.h"
#include "Public/Tables/MVSkillDataTableColumn.h"
#include "Public/Enum/MVCombatActionTypes.h"
#include "Public/Enum/MVSkillSlot.h"
#include "Components/MVInputManagerComponent.h"
#include "Interface/MVActionInputHandlerInterface.h"
#include "Struct/MVHitTypes.h"

#include "MVCombatComponent.generated.h"

class UMVAbilityBase;

USTRUCT(BlueprintType)
struct FMVCombatActionEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Event")
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Event")
	EMVCombatActionTypes ActionType = EMVCombatActionTypes::None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Event")
	EMVSkillSlot SkillSlot = EMVSkillSlot::NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Event")
	FName ActionTableName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Event")
	FName ActionRowName = NAME_None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnCombatActionEvent, const FMVCombatActionEvent&, Event);

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

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	bool bCurrentStageHitConfirmed = false;

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
		bCurrentStageHitConfirmed = false;
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
			bCurrentStageHitConfirmed = false;

			const FMVSkillDataTableColumn* CurrentData = GetCurrentSkillData();
			if (CurrentData)
			{
				InputWindowCloseTime = CurrentTime
					+ FMath::Max(0.0f, CurrentData->InterStageCooldown)
					+ FMath::Max(0.0f, CurrentData->InputWindowDuration);
			}
		}
		// if Simple Skill or invalid Skill
		else
		{
			bChainActive = false;
			LastUsedTime = CurrentTime;
			bCurrentStageHitConfirmed = false;
			return;
		}
	}

	void StartPostAbilityResetWindow(float CurrentTime)
	{
		if (bChainActive)
		{
			const FMVSkillDataTableColumn* CurrentData = GetCurrentSkillData();

			if (!CurrentData)
			{
				return;
			}

			LastStageActivationTime = CurrentTime;

			if (CurrentData->ChainAdvancePolicy == EMVChainAdvancePolicy::OnHitConfirmed)
			{
				return;
			}

			if (CurrentData)
			{
				InputWindowCloseTime = CurrentTime
					+ FMath::Max(0.0f, CurrentData->InterStageCooldown)
					+ FMath::Max(0.0f, CurrentData->InputWindowDuration);
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
			bCurrentStageHitConfirmed = false;
			LastUsedTime = CurrentTime;
			CurrentChainStageIndex = 0;
			return false; // Chain complete
		}

		CurrentChainStageIndex++;
		LastStageActivationTime = CurrentTime;
		bCurrentStageHitConfirmed = false;

		const FMVSkillDataTableColumn* CurrentData = GetCurrentSkillData();
		if (CurrentData)
		{
			InputWindowCloseTime = CurrentTime
				+ FMath::Max(0.0f, CurrentData->InterStageCooldown)
				+ FMath::Max(0.0f, CurrentData->InputWindowDuration);
		}

		return true;
	}
};

/** Read-only UI projection of one player skill slot. */
USTRUCT(BlueprintType)
struct FMVSkillSlotRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bLoadoutAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bGaugeControlled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bGaugeReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bUsable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bChainActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bOnCooldown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	bool bChainTimerVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	int32 ActiveStackIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	int32 StackSize = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	int32 ChainTimerIconIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	float CooldownRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	float CooldownDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	float ChainWindowRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	float ChainWindowDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	float GaugeCurrent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	float GaugeMax = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Combat|Skill UI")
	float GaugeRatio = 0.0f;
};

class UMVStatComponent;

struct FMVCombatHeavyChargeAttackRuntimeState
{
	bool bActive = false;
	bool bCommitted = false;
	float StartedWorldTime = 0.0f;
	float CommitTime = 0.0f;
	FName ChargeActionTableName = NAME_None;
	EMVCombatActionTypes ChargeActionType = EMVCombatActionTypes::None;
	int32 ChargeChainStageIndex = INDEX_NONE;
	FDataTableRowHandle ChargeRowHandle;
};

/**
 * 캐릭터의 공격·실행형 스킬과 장착 Passive Ability 런타임을 소유하는 전투 컴포넌트.
 *
 * 실행형 Ability는 Action·AnimNotify 흐름에서 시작·종료하고, Passive Ability는 별도 슬롯에서
 * ApplyPassiveSkill과 ClearPassiveSkill로 직접 시작·종료한다. Passive 교체 요청은 기존 인스턴스를
 * 먼저 해제하며 신규 적용 실패 시 공석을 정상 상태로 유지한다. EndPlay에서는 활성 Passive를
 * 반드시 종료하지만 사망만으로는 해제하지 않는다.
 * 무기별 Basic Attack과 장착 Skill Row는 WeaponComponent를 단일 진입점으로 조회한다.
 * 무기 변경은 전체 액션 맵을 다시 만들고, 같은 스타일의 스킬 교체는 해당 슬롯만 갱신한다.
 */
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

	UFUNCTION(BlueprintCallable, Category = "Maverick|Combat|Action")
	bool TryBasicCombatAction(EMVCombatActionTypes InActionType, FName StartSection = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Combat|Action")
	bool TrySkillCombatAction(EMVSkillSlot SkillSlot, FName StartSection = NAME_None);

	/** AI가 선택한 전투 행의 Ability 준비와 액션 시작 진입점; 실행 중인 액션 교체 제외 */
	UFUNCTION(BlueprintCallable, Category = "Action")
	bool TryStartCombatActionFromRowHandle(const FDataTableRowHandle& RowHandle, FName StartSection = NAME_None);

	UFUNCTION(BlueprintPure, Category = "Maverick|Combat|Skill UI")
	bool GetSkillSlotRuntimeState(EMVSkillSlot SkillSlot, FMVSkillSlotRuntimeState& OutState) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Combat|Passive Skill")
	bool ApplyPassiveSkill(const FDataTableRowHandle& PassiveSkillRowHandle);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Combat|Passive Skill")
	void ClearPassiveSkill();

	UFUNCTION(BlueprintPure, Category = "Maverick|Combat|Passive Skill")
	UMVAbilityBase* GetActivePassiveSkill() const { return ActivePassiveAbilityInstance.Get(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|Combat|Passive Skill")
	FDataTableRowHandle GetActivePassiveSkillRowHandle() const { return ActivePassiveSkillRowHandle; }

	void HandleAbilityEnded(const UMVAbilityBase* EndedAbility);

	UFUNCTION()
	void HandleHitResolved(const FMVResolvedHitData& HitData);

	// Call When Character Change Weapon --> have to receive Event from Character
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void ChangeWeapon(EMVEquippedStyle NewStyle);

	void RefreshSkillSlot(EMVSkillSlot SkillSlot);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Combat|Event")
	FMVOnCombatActionEvent OnCombatActionStarted;

protected:
	virtual bool TryHandleActionInput(FGameplayTag ActionInputTag, FVector2D ControllerSpaceInput, bool bHasMovementInput) override;
	virtual bool TryHandleHoldActionInput(
		FGameplayTag ActionInputTag,
		EMVActionInputPhase Phase,
		float HeldSeconds,
		FVector2D ControllerSpaceInput,
		bool bHasMovementInput) override;
	bool ChooseTryCombatAction(FGameplayTag ActionInputTag);
	bool IsCombatActionInputTag(FGameplayTag ActionInputTag) const;
	bool IsHeavyChargeActionInputTag(FGameplayTag ActionInputTag) const;

	bool CanStartCombatAction() const;
	bool TryBasicAttack(EMVCombatActionTypes InActionType, FName StartSection = NAME_None);
	bool TrySkill(EMVSkillSlot SkillSlot, FName StartSection = NAME_None);

	// Call when Beginplay or Change Weapon Style
	void RefreshActionMaps();
	
	void ResetBasicAttackMap();
	void ResetSkillMap();
private:
	// AI 직접 실행 Ability의 GC 보존; 다음 직접 액션 시작 시 교체
	UPROPERTY(Transient)
	FMVSkillEntry DirectActionEntry;

	bool BuildSkillEntryFromRowHandle(const FDataTableRowHandle& StartingRowHandle, FMVSkillEntry& OutEntry);
	bool IsValidSkillActionRowHandle(const FDataTableRowHandle& RowHandle, const TCHAR* Context) const;
	bool IsCurrentAbilityAction(FName ActionTableName, FName ActionRowName) const;
	bool TryStartActionWithAbility(
		FMVSkillEntry& ActionEntry,
		const FDataTableRowHandle& RowHandle,
		FName StartSection = NAME_None,
		bool bForceTransition = false,
		float TransitionBlendOutTime = 0.25f);
	bool IsRSkillGaugeReady() const;
	bool IsCurrentAbilityRSkill() const;
	void AddRSkillGauge(float GainMultiplier);
	void ResetRSkillGauge();

	UFUNCTION()
	void HandleOwnerDeathStarted(const FMVDeathContext& DeathContext);
	bool CanConsumeActionCost(const FMVSkillDataTableColumn* SkillData) const;
	bool IsBasicAttackActionType(EMVCombatActionTypes ActionType) const;
	int32 SelectBasicAttackChainStageForSwing(const FMVSkillEntry& ActionEntry) const;
	const FMVSkillDataTableColumn* GetBasicAttackSkillDataAtStage(const FMVSkillEntry& ActionEntry, int32 ChainStageIndex) const;
	void UpdateLastBasicAttackSwingDirection(const FDataTableRowHandle& RowHandle);
	void ClearLastBasicAttackSwingDirection();
	void ResetOtherBasicAttackChains(EMVCombatActionTypes ExceptActionType);
	void MarkBasicAttackChainStarted(
		FMVSkillEntry& ActionEntry,
		EMVCombatActionTypes ActionType,
		int32 StartedChainStageIndex,
		float CurrentTime);
	bool IsHeavyChargeBasicAttackType(EMVCombatActionTypes ActionType) const;
	int32 ResolveHeavyChargeStartChainStageIndex(const FMVSkillEntry& ChargeEntry) const;
	int32 ResolveHeavyChargeEarlyReleaseChainStageIndex(const FMVSkillEntry& HeavyEntry) const;
	bool TryBeginHeavyChargeAttack(float HeldSeconds);
	bool TryUpdateHeavyChargeAttack(float HeldSeconds);
	bool TryReleaseHeavyChargeAttack(float HeldSeconds, bool bCanceled);
	bool TryCommitHeavyChargeAttack(float HeldSeconds);
	bool TryEarlyReleaseHeavyChargeAttack(float HeldSeconds);
	bool IsHeavyChargeActionRunning() const;
	float ResolveHeavyChargeCommitTime(const FMVSkillDataTableColumn* ChargeSkillData) const;
	bool ResolveHeavyChargeEarlyReleaseRow(
		FMVSkillEntry*& OutHeavyEntry,
		FDataTableRowHandle& OutHeavyRowHandle,
		int32& OutHeavyChainStageIndex);
	void ResetHeavyChargeAttackState();
	EMVCombatActionTypes ResolveContextualBasicAttackActionType(EMVCombatActionTypes RequestedActionType);
	void MarkContextualBasicAttackStarted(EMVCombatActionTypes StartedActionType);
	void BroadcastCombatActionStarted(
		EMVCombatActionTypes ActionType,
		EMVSkillSlot SkillSlot);
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
	TMap<EMVCombatActionTypes, FMVSkillEntry>BasicAttackMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Logic")
	TMap<EMVSkillSlot, FMVSkillEntry>SkillMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	EMVEquippedStyle CurrentWeaponStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Infomation")
	float ResetBasicAttackTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Sprint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SprintAttackMinSpeedRatio = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|HeavyCharge", meta = (ClampMin = "0.0", Units = "s"))
	float DefaultHeavyChargeCommitTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|HeavyCharge")
	FName HeavyChargeStartSection = TEXT("LeadIn");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|R Skill Gauge")
	bool bUseRSkillGauge = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|R Skill Gauge", meta = (ClampMin = "0.01"))
	float RSkillGaugeMax = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|R Skill Gauge", meta = (ClampMin = "0.0"))
	float RSkillGaugeGainPerHit = 10.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMVAbilityBase> PreviousAbilityInstance;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVAbilityBase> CurrentAbilityInstance;

	FName CurrentAbilityActionTableName = NAME_None;
	FName CurrentAbilityActionRowName = NAME_None;
	bool bCurrentAbilityAwaitingCompletion = false;

	int32 NextAttackInstanceId = 0;
	int32 CurrentAttackInstanceId = INDEX_NONE;

private:
	double LastBasicAttackedTime = 0.0;
	EMVCombatActionTypes ActiveBasicAttackType = EMVCombatActionTypes::None;
	TObjectPtr<UMVStatComponent> StatComponent;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Combat|Passive Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVAbilityBase> ActivePassiveAbilityInstance;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Combat|Passive Skill", meta = (AllowPrivateAccess = "true"))
	FDataTableRowHandle ActivePassiveSkillRowHandle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|R Skill Gauge", meta = (AllowPrivateAccess = "true"))
	float CurrentRSkillGauge = 0.0f;

	bool bSprintContextualBasicAttackConsumed = false;
	bool bWasSprintAttackContextActive = false;
	int32 ConsumedDodgeContextActionInstanceId = INDEX_NONE;
	int32 PendingDodgeContextActionInstanceId = INDEX_NONE;
	EMVAttackSwingDirection LastBasicAttackSwingDirection = EMVAttackSwingDirection::None;
	FMVCombatHeavyChargeAttackRuntimeState HeavyChargeAttackState;

};
