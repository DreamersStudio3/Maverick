#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Struct/MVHitTypes.h"
#include "Misc/Guid.h"
#include "MVStatComponent.generated.h"

class UMVTableManager;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnStatValueChanged, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnStatRecentLossHoldChanged, bool, bHold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnGroggyStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnGroggyEnded);

UENUM(BlueprintType)
enum class EMVDeathReason : uint8
{
	// CurrentHP가 0 이하로 감소해 사망한 정상 사망 경로.
	HPDepleted,

	// 정상 게임 규칙에서는 발생하지 않아야 한다. MaxHP는 최소 1까지 감소할 수 있으므로,
	// 최대 HP 감소만으로 CurrentHP가 0이 되는 상황은 비정상 데이터/디버그용으로만 구분한다.
	MaxHPReduced
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVDeathContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Death")
	TObjectPtr<AActor> DeadActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Death")
	EMVDeathReason Reason = EMVDeathReason::HPDepleted;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Death")
	bool bHasHitData = false;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Death")
	FMVResolvedHitData HitData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnDeathStarted, const FMVDeathContext&, DeathContext);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMVOnModifyIncomingDamage, const FMVResolvedHitData&, float&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FMVOnDamageApplied,
	float, AppliedDamage,
	float, PreviousHP,
	float, CurrentHP,
	const FMVResolvedHitData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FMVOnDamageAccumulated,
	float, AccumulatedDamage,
	float, AppliedDamage,
	float, PreviousHP,
	float, CurrentHP,
	const FMVResolvedHitData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnDamageAccumulationReset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnBaseStatsReady, int32, Revision);

/**
 * CharacterIndexCode와 일치하는 스탯 행의 HP·스태미너·MP·그로기·이동·전투 수치와 변경 이벤트 소유
 * BeginPlay에서 기본 스탯 로드, Consume 계열·OnDamaged에서 확정 피해 차감
 * 최초 HP 소진 시 OnDeathStarted(FMVDeathContext) 발행, Blueprint 호환용 OnDead 병행
 * TickRecoverableStats에서 자원·Exhaustion·그로기 회복, Notify의 회복 정지와 최근 감소 UI 홀드 관리
 * 강인도 소진 구간은 Begin/EndPoiseBreakGroggyWindow로 기존 수치·회복 타이머 보관·복원
 * 해당 구간을 시작한 호출자의 종료 책임, 피해 계산·피격 표현·캐릭터 선택 정책은 외부 도메인 책임
 */

UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVStatComponent();
	
	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnStatValueChanged OnHPChanged;

	FMVOnModifyIncomingDamage OnModifyIncomingDamage;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnDamageApplied OnDamageApplied;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnDamageAccumulated OnDamageAccumulated;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnDamageAccumulationReset OnDamageAccumulationReset;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnStatValueChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnStatValueChanged OnMPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnStatValueChanged OnGroggyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnStatRecentLossHoldChanged OnStatRecentLossHoldChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnDead OnDead;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnGroggyStarted OnGroggyStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnGroggyEnded OnGroggyEnded;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnDeathStarted OnDeathStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnBaseStatsReady OnBaseStatsReady;
	
protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Table")
	void SetStatTableReference(FName InStatTableName, const FString& InStatRowKey);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Character")
	void SetCharacterIndexCode(FGameplayTag NewCharacterIndexCode);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Character")
	FGameplayTag GetCharacterIndexCode() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Table")
	bool LoadStatsFromTable();

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Progression")
	bool IsBaseStatsReady() const { return bBaseStatsReady; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Progression")
	int32 GetBaseStatsRevision() const { return BaseStatsRevision; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Progression")
	float GetBaseMaxHP() const { return BaseMaxHP; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Progression")
	float GetBaseMaxStamina() const { return BaseMaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Progression")
	float GetBaseMaxMP() const { return BaseMaxMP; }
	
	bool ReplaceProgressionStatBonuses(const TMap<FGameplayTag, float>& InBonuses);

	bool TryGetProgressionStatValues(
		const FGameplayTag& StatId,
		float& OutBaseValue,
		float& OutBonus,
		float& OutEffectiveValue) const;

	int32 GetStatCalculationRevision() const { return StatCalculationRevision; }

	void ApplyIncomingDamageModifiers(FMVResolvedHitData& HitData);
	
	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Damage")
	void HandleDamaged(const FMVResolvedHitData& HitData);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Death")
	bool WouldDieFromHit(const FMVResolvedHitData& HitData) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Death")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Groggy")
	bool IsGroggy() const { return bIsGroggy; }

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Death")
	void ResetDeathState();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void ResetGroggyState();

	void TickRecoverableStats(float DeltaTime, bool bAllowStaminaRecovery);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Recovery")
	void BeginRecoverableStatRecoveryPause();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Recovery")
	void EndRecoverableStatRecoveryPause();

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Recovery")
	bool IsRecoverableStatRecoveryPaused() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|HP")
	void SetMaxHP(float InMaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|HP")
	void SetCurrentHP(float InCurrentHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|HP")
	void RecoverHP(float Amount);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|HP")
	bool CanConsumeHP(float Amount) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|HP")
	bool ConsumeHP(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetMaxStamina(float InMaxStamina);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetCurrentStamina(float InCurrentStamina);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetStaminaRecoveryPerSecond(float InStaminaRecoveryPerSecond);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetStaminaRecoveryDelay(float InStaminaRecoveryDelay);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Stamina")
	bool IsStaminaExhausted() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Stamina")
	bool HasStamina(float RequiredAmount) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Stamina")
	bool HasAnyStamina() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	bool ConsumeStaminaAllowPartial(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void RecoverStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MP")
	void SetMaxMP(float InMaxMP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MP")
	void SetCurrentMP(float InCurrentMP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MP")
	void SetMPRecoveryPerSecond(float InMPRecoveryPerSecond);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|MP")
	bool HasMP(float RequiredAmount) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MP")
	bool ConsumeMP(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MP")
	void RecoverMP(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Attack")
	void SetAttackPower(float InAttackPower);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Attack")
	void SetAttackSpeed(float InAttackSpeed);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Attack")
	float GetBaseAttackSpeed() const { return BaseAttackSpeed; }

	// 증가율을 등록하고, 이후 갱신·제거에 사용할 고유 번호 반환
	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Attack")
	FGuid AddAttackSpeedModifier(float BonusRatio);

	// 같은 효과의 증가율을 새 값으로 교체
	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Attack")
	bool UpdateAttackSpeedModifier(FGuid ModifierHandle, float BonusRatio);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Attack")
	bool RemoveAttackSpeedModifier(FGuid ModifierHandle);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	FGuid AddMoveSpeedModifier(float BonusRatio);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	bool UpdateMoveSpeedModifier(FGuid ModifierHandle, float BonusRatio);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	bool RemoveMoveSpeedModifier(FGuid ModifierHandle);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|MoveSpeed")
	float GetMoveSpeedMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Dodge")
	FGuid AddDodgeStaminaCostModifier(float BonusRatio);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Dodge")
	bool RemoveDodgeStaminaCostModifier(FGuid ModifierHandle);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Dodge")
	float GetDodgeStaminaCostMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetWalkSpeed(float InWalkSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetRunSpeed(float InRunSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetSprintSpeed(float InSprintSpeed);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|MoveSpeed")
	bool HasReachedSprintSpeedRatio(float RequiredRatio) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Defence")
	void SetNormalDefence(float NewNormalDefence);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Defence")
	void SetSkillDefence(float NewSkillDefence);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetMaxGroggy(float InMaxGroggy);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetCurrentGroggy(float InCurrentGroggy);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void RecoverGroggy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetGroggyRecoveryPerSecond(float InGroggyRecoveryPerSecond);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Damage")
	void SetRecentDamageResetDelay(float InRecentDamageResetDelay);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy", meta = (DeprecatedFunction, DeprecationMessage = "Use SetRecentDamageResetDelay."))
	void SetGroggyRecoveryDelay(float InGroggyRecoveryDelay);

	// Poise
	UFUNCTION(BlueprintCallable, Category = "Maverick|Poise")
	void SetInitialPoise(float InitialPoise);
	
	// 행동 시작/종료 시 상시 강인도, 무기 강인도 조정
	UFUNCTION(BlueprintCallable, Category = "Maverick|Poise")
	void PoiseActionStart(float WeaponPoise, float Multiplier);
	UFUNCTION(BlueprintCallable, Category = "Maverick|Poise")
	void PoiseActionEnd();
	
	// 강인도 수치 변화 함수(강인도 수치를 변화시키고 Poise Break를 판정)
	UFUNCTION(BlueprintCallable, Category = "Maverick|Poise")
	void UpdatePoise(float PoiseDamageAmount);

	// 강인도 수치 초기화 함수(강인도 수치를 초기값으로 되돌림)
	UFUNCTION(BlueprintCallable, Category = "Maverick|Poise")
	void ResetPoise();

	// 강인도 수치가 Poise Break를 일으킬지 예측하는 함수 -> HitResolverSubsystem에서 사용
	bool PredictPoiseBreak(float PoiseDamageAmount) const;

	// 한정된 강인도 소진 구간: 회복 정지·추가 강인도 제외, 종료 시 기존 수치·회복 타이머 복원
	void BeginPoiseBreakGroggyWindow(float Poise);
	void EndPoiseBreakGroggyWindow();
	bool IsPoiseBreakGroggyWindowActive() const { return bPoiseBreakGroggyWindow; }

private:
	FString MakeStatRowKey() const;
	void TickRecentDamageCooldown(float DeltaTime);
	void TickGroggyRecovery(float DeltaTime);
	void TickRecoverableResourceRecovery(float DeltaTime);
	void BeginStaminaExhaustion();
	void TickStaminaExhaustion(float DeltaTime);
	void BroadcastDeathStarted(EMVDeathReason Reason);
	void RestartRecentDamageCooldown();
	void ResetDamageAccumulation();
	bool TryStartGroggy();
	void BroadcastGroggyEnded();
	void ApplyProgressionStatBonuses();

	// 추가 강인도 설정(무기, 행동 등)
	void SetAdditionalPoise(float WeaponPoise = 0, float Multiplier = 0);
	
	// 행동 시작 시, 상시 강인도의 최저 하한선 체크, 조정
	void AdjustMinPoise();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Table")
	FName StatTableName = TEXT("CharacterStat");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Character", meta = (Categories = "Character"))
	FGameplayTag CharacterIndexCode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Table")
	bool bLoadStatsOnBeginPlay = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|HP")
	float MaxHP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|HP")
	float CurrentHP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Stamina")
	float CurrentStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Stamina")
	float StaminaRecoveryPerSecond = 25.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Stamina")
	float StaminaRecoveryDelay = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float MaxMP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float CurrentMP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float MPRecoveryPerSecond = 5.0f;

	// 캐릭터 기본 공격력, 일반공격은 해당 수치를, 스킬 공격은 스킬 배율을 곱한 수치를 최종 대미지로 사용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Attack")
	float AttackPower = 10.0f;

	// 공격 속도, 1.0은 기본 속도, 2.0은 2배 속도, 0.5는 절반 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Attack")
	float AttackSpeed = 1.0f;

	// 치명타 확률, 0.0~1.0 사이의 값으로 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Attack")
	float CriticalPercent = 0.0f;

	// 치명타 데미지, 1.0은 기본 데미지, 2.0은 2배 데미지, 0.5는 절반 데미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Attack")
	float CriticalDamage = 1.0f;

	// 기본공격의 흡혈 비율, 1.0은 100% 흡혈, 0.5는 50% 흡혈, 0.0은 흡혈 없음
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Attack")
	float NormalVamp = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MoveSpeed")
	float WalkSpeed = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MoveSpeed")
	float RunSpeed = 500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MoveSpeed")
	float SprintSpeed = 750.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Groggy")
	float MaxGroggy = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Groggy")
	float CurrentGroggy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Groggy")
	float GroggyRecoveryPerSecond = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Damage")
	float RecentDamageResetDelay = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Damage")
	float AccumulatedRecentDamage = 0.0f;

	// NormalDefence는 공격자가 입힌 평타 대미지를 얼마나 감소시키는지에 대한 수치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Defence")
	float NormalDefence = 10.0f;

	// SkillDefence는 공격자가 입힌 스킬 대미지를 얼마나 감소시키는지에 대한 수치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Defence")
	float SkillDefence = 5.0f;
	
	// Poise는 피격 시 경직을 얼마나 잘 버티는지에 대한 수치로, 공격자에게 밀려나거나 넘어지는 피격 반응을 결정하는 데 사용된다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Poise")
	float InitialCharacterPoise = 0.0f;
	
	// 상시 강인도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Poise")
	float ConstantPoise = 0.0f;

	// 무기, 행동의 강인도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Poise")
	float AdditionalPoise = 0.0f;
	
	// 행동(공격) 시 최저 하한선 강인도 비율, 이 비율보다 낮으면 행동 시 상시강인도를 조정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Poise")
	float MinPoiseRecoveryRatio = 0.8f;
	
	// 강인도 초기화 기준 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Poise")
	float PoiseRecoveryTime = 30.0f;

private:
	void RecalculateAttackSpeed();

	float RecentDamageCooldownRemaining = 0.0f;
	int32 RecoverableStatRecoveryPauseCount = 0;
	float StaminaExhaustionRemaining = 0.0f;
	FMVResolvedHitData PendingDeathHitData;
	bool bHasPendingDeathHitData = false;
	bool bHasRecentDamageAccumulation = false;
	bool bIsDead = false;
	bool bIsGroggy = false;

	// Poise
	FTimerHandle PoiseRecoveryTimerHandle;
	bool bPoiseBreakGroggyWindow = false;
	float SavedWindowConstantPoise = 0.0f;
	float SavedWindowAdditionalPoise = 0.0f;
	float SavedWindowRecoveryRemaining = -1.0f;
	
	TMap<FGameplayTag, float> ProgressionStatBonuses;
	int32 StatCalculationRevision = 0;
	
	float BaseMaxHP = 100.0f;
	float BaseMaxStamina = 100.0f;
	float BaseMaxMP = 100.0f;
	int32 BaseStatsRevision = 0;
	bool bBaseStatsReady = false;

	UPROPERTY(Transient)
	float BaseAttackSpeed = 1.0f;

	UPROPERTY(Transient)
	TMap<FGuid, float> AttackSpeedModifiers;

	UPROPERTY(Transient)
	TMap<FGuid, float> MoveSpeedModifiers;

	UPROPERTY(Transient)
	TMap<FGuid, float> DodgeStaminaCostModifiers;
};
