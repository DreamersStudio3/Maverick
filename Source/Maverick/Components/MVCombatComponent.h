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
class AMVProjectileBase;

// 기존 Ability와 패시브가 사용하는 Q/R/W/E 번호. 새 맵의 키와는 별개다.
namespace MVCombatSkillSlots
{
	inline constexpr int32 Q = 0;
	inline constexpr int32 R = 1;
	inline constexpr int32 W = 2;
	inline constexpr int32 E = 3;
	inline constexpr int32 Count = 4;
}

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnAttackMissed, UMVAbilityBase*, MissedAbility);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnValidatedAttackHit, const FMVResolvedHitData&, HitData, UMVAbilityBase*, Ability);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMVOnModifyOutgoingAttackDamage, const FMVResolvedHitData&, float&);

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

	// 현재 슬롯 구성과 연계 전체를 묶는 사용 회차. 투사체의 지연 적중 출처로 복사한다.
	uint64 LoadoutRevision = 0;
	uint64 CurrentUseId = 0;

	// Main cooldown duration (applies to entire skill or chain)
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float MainCooldownDuration = 0.0f;

	// Last time this skill was used
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	float LastUsedTime = 0.0f;

	// 음수는 아직 주 쿨타임을 시작하지 않았다는 뜻이다.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	float MainCooldownEndTime = -1.0f;

	// 이번 스킬 사용에서 주 쿨타임 환원이 성공했는지 기록한다.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	bool bMainCooldownRefundedForCurrentUse = false;

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

	void BeginMainCooldown(float CurrentTime)
	{
		LastUsedTime = CurrentTime;
		MainCooldownEndTime = bMainCooldownRefundedForCurrentUse
			? CurrentTime
			: CurrentTime + FMath::Max(0.0f, MainCooldownDuration);
	}

	float GetMainCooldownRemaining(float CurrentTime) const
	{
		if (MainCooldownDuration <= 0.0f || MainCooldownEndTime < 0.0f)
		{
			return 0.0f;
		}

		return FMath::Clamp(
			MainCooldownEndTime - CurrentTime,
			0.0f,
			MainCooldownDuration);
	}

	// Check if main cooldown is ready
	bool IsMainCooldownReady(float CurrentTime) const
	{
		return GetMainCooldownRemaining(CurrentTime) <= KINDA_SMALL_NUMBER;
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
			BeginMainCooldown(CurrentTime);
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

		BeginMainCooldown(CurrentTime);
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
			BeginMainCooldown(CurrentTime);
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
 * 한 공격에서 발사한 탄과 원래 스킬 사용 회차를 묶는 임시 기록.
 * 첫 탄 등록 때 행·슬롯·무기·생존 회차를 고정하고, 환원 성공도 공격별로 기록한다.
 * 모든 탄과 동작이 끝난 뒤 제거한다.
 * 실제로 활성화한 탄이 하나도 없으면 빗나감 사건을 내지 않는다.
 */
USTRUCT()
struct FMVProjectileExecutionRecord
{
	GENERATED_BODY()

	TWeakObjectPtr<UMVAbilityBase> SourceAbility;

	UPROPERTY(Transient)
	TObjectPtr<UMVAbilityBase> ExecutionAbility = nullptr;

	TSet<AMVProjectileBase*> ActiveProjectiles;
	UPROPERTY(Transient)
	FDataTableRowHandle SourceRowHandle;
	EMVSkillSlot SourceSkillSlot = EMVSkillSlot::NONE;
	uint64 SourceSlotRevision = 0;
	uint64 SourceSkillUseId = 0;
	uint64 SourceLifeId = 0;
	FGameplayTag SourceWeaponItemTag;
	uint64 PassiveActivationId = 0;
	int32 SourceChainStageIndex = INDEX_NONE;
	bool bWasBasicAttack = false;
	bool bWasRSkill = false;
	bool bHadActivatedProjectile = false;
	bool bHadHit = false;
	bool bMissPending = false;
	bool bMissNotified = false;
	bool bActionEnded = false;
	bool bRefundProcessed = false;
};

/**
 * 캐릭터의 공격·실행형 스킬과 장착 Passive Ability 런타임을 소유하는 전투 컴포넌트.
 *
 * 실행형 Ability는 Action·AnimNotify 흐름에서 시작·종료하고, Passive Ability는 별도 슬롯에서
 * ApplyPassiveSkill과 ClearPassiveSkill로 직접 시작·종료한다. Passive 교체 요청은 기존 인스턴스를
 * 먼저 해제하며 신규 적용 실패 시 공석을 정상 상태로 유지한다. EndPlay에서는 활성 Passive를
 * 반드시 종료하지만 사망만으로는 해제하지 않는다.
 * 무기별 Basic Attack과 장착 Skill Row는 WeaponComponent를 단일 진입점으로 조회한다.
 * 스타일 변경은 전체 액션 맵을, 아이템·종류 변경은 패시브 슬롯을 갱신한다.
 * 공격 시작 때 패시브 적용 식별값을 보관해 교체 전 공격이 새 패시브를 발동하지 않게 한다.
 * 투사체는 발사 시점의 스킬 행·구성·사용·생존 회차와 함께 기록해 동작 종료 후에도 정리한다.
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

	// 공격이 시작될 때 활성화된 패시브가 현재도 같은 실행 객체인지 확인한다.
	bool IsAttackFromCurrentPassiveActivation(const FMVResolvedHitData& HitData) const;
	bool IsAttackFromCurrentPassiveActivation(const UMVAbilityBase* Ability) const;

	// 진행 중인 주 쿨타임의 남은 시간을 횟수만큼 비율로 줄인다.
	UFUNCTION(BlueprintCallable, Category = "Maverick|Combat|Cooldown")
	bool ReduceSkillMainCooldown(
		int32 SkillIndex,
		float ReductionFraction,
		int32 TriggerCount,
		float& OutBeforeSeconds,
		float& OutAfterSeconds);

	// 현재 무기에서 주 쿨타임이 진행 중인 스킬에만 감소를 적용한다.
	int32 ReduceOngoingSkillMainCooldowns(
		float ReductionFraction,
		int32 TriggerCount);

	// 같은 스킬 사용 회차의 공격에 한 번만 주 쿨타임 환원을 승인한다.
	// 성공한 호출만 MP 환원과 짝을 이루며, 이미 끝난 쿨타임도 유효한 사용으로 취급한다.
	bool RefundSkillMainCooldownForExecution(
		int32 SkillIndex,
		const UMVAbilityBase* SourceAbility);

	void HandleAbilityEnded(const UMVAbilityBase* EndedAbility);

	// 같은 동작의 다음 발사 Notify가 시작되면 종료 대기 상태를 다시 연다.
	void NotifyProjectileAbilityStarted(const UMVAbilityBase* StartedAbility);

	// 발사체의 충돌을 켜기 전에 호출한다.
	bool RegisterLaunchedProjectile(
		AMVProjectileBase* Projectile,
		UMVAbilityBase* SourceAbility);

	UFUNCTION()
	void HandleHitResolved(const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleHitDeliveryFinished(const FMVResolvedHitData& HitData);

	// 실행된 Ability가 현재 무기의 평타 목록에 속하는지 확인한다.
	bool IsBasicAttackAbility(const UMVAbilityBase* Ability) const;

	void ApplyOutgoingAttackDamageModifiers(FMVResolvedHitData& HitData);

	// Call When Character Change Weapon --> have to receive Event from Character
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void ChangeWeapon(EMVEquippedStyle NewStyle);

	void RefreshSkillSlot(EMVSkillSlot SkillSlot);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Combat|Event")
	FMVOnCombatActionEvent OnCombatActionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Combat|Event")
	FMVOnAttackMissed OnAttackMissed;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Combat|Event")
	FMVOnValidatedAttackHit OnValidatedAttackHit;

	// 원래 타격의 피해자 처리가 끝난 유효 공격 명중
	UPROPERTY(BlueprintAssignable, Category = "Maverick|Combat|Event")
	FMVOnValidatedAttackHit OnValidatedAttackHitAfterDamage;

	FMVOnModifyOutgoingAttackDamage OnModifyOutgoingAttackDamage;

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
	bool IsCurrentAttackHit(const FMVResolvedHitData& HitData) const;
	bool IsProjectileSourceWeaponEquipped(
	const FMVProjectileExecutionRecord& Record,
	const FMVResolvedHitData& HitData) const;
	void CloseProjectileExecution(int32 AttackInstanceId);
	void FinalizeProjectileExecution(int32 AttackInstanceId);

	UFUNCTION()
	void HandleRegisteredProjectileHit(
		AMVProjectileBase* Projectile,
		const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleRegisteredProjectileFinished(
		AMVProjectileBase* Projectile,
		int32 AttackInstanceId);

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
	UPROPERTY(Transient)
	FDataTableRowHandle CurrentAbilityRowHandle;
	EMVSkillSlot CurrentPreparedSkillSlot = EMVSkillSlot::NONE;
	int32 CurrentPreparedChainStageIndex = INDEX_NONE;
	uint64 CurrentPreparedSlotRevision = 0;
	uint64 CurrentPreparedSkillUseId = 0;
	bool bCurrentAbilityAwaitingCompletion = false;

	int32 NextAttackInstanceId = 0;
	int32 CurrentAttackInstanceId = INDEX_NONE;
	uint64 NextSkillSlotRevision = 0;
	uint64 NextSkillUseId = 0;
	uint64 CurrentLifeId = 1;
	uint64 CurrentAttackLifeId = 0;
	uint64 CurrentAttackPassiveActivationId = 0;
	int32 CurrentRefundedAttackInstanceId = INDEX_NONE;

	bool bCurrentAbilityHitConfirmed = false;

private:
	double LastBasicAttackedTime = 0.0;
	friend class FMVProjectileExecutionTest;
	EMVCombatActionTypes ActiveBasicAttackType = EMVCombatActionTypes::None;
	TObjectPtr<UMVStatComponent> StatComponent;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Combat|Passive Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMVAbilityBase> ActivePassiveAbilityInstance;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Combat|Passive Skill", meta = (AllowPrivateAccess = "true"))
	FDataTableRowHandle ActivePassiveSkillRowHandle;

	uint64 NextPassiveActivationId = 0;
	uint64 ActivePassiveActivationId = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|R Skill Gauge", meta = (AllowPrivateAccess = "true"))
	float CurrentRSkillGauge = 0.0f;

	bool bSprintContextualBasicAttackConsumed = false;
	bool bWasSprintAttackContextActive = false;
	int32 ConsumedDodgeContextActionInstanceId = INDEX_NONE;
	int32 PendingDodgeContextActionInstanceId = INDEX_NONE;
	EMVAttackSwingDirection LastBasicAttackSwingDirection = EMVAttackSwingDirection::None;
	FMVCombatHeavyChargeAttackRuntimeState HeavyChargeAttackState;

	UPROPERTY(Transient)
	TMap<int32, FMVProjectileExecutionRecord> ProjectileExecutions;

};
