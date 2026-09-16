#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Struct/MVHitTypes.h"
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

/**
 * 캐릭터 스탯 값과 회복 정책을 관리하는 컴포넌트.
 *
 * 명시적으로 설정된 CharacterIndexCode와 동일한 CharacterStat row에서 기본 스탯을
 * 로드하고 HP, 스태미너, MP, groggy, 이동/전투 수치의 현재값과 변경 이벤트를 소유한다.
 * `OnDamaged` 구독을 통해 확정된 피해의 HP 차감도 처리한다.
 * HP가 처음 0 이하가 되면 문맥을 담은 `OnDeathStarted(FMVDeathContext)`를 사망 진입점으로 발행하고,
 * 매개변수 없는 `OnDead`는 기존 Blueprint와의 호환을 위해 함께 유지한다.
 * NotifyState가 요청한 회복 일시정지, 최근 감소 UI 홀드, 그로기 누적 게이지 감소도 이 컴포넌트의 상태로 관리한다.
 * 다른 도메인 컴포넌트의 캐릭터 선택 상태는 참조하지 않는다.
 *
 * 라이프사이클:
 *   1) BeginPlay -> 설정된 스탯 테이블 행을 읽어 현재 스탯 값을 초기화한다.
 *   2) ConsumeStamina/ConsumeMP -> 값을 감소시킨다.
 *   3) TickRecoverableStats -> 외부 이동/액션 정책이 회복 가능한 프레임에 호출해 스태미너와 MP를 회복한다.
 */

UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVStatComponent();
	
	UPROPERTY(BlueprintAssignable, Category = "Maverick|Stat|Event")
	FMVOnStatValueChanged OnHPChanged;

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

	void TickRecoverableStats(float DeltaTime);

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

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetMaxStamina(float InMaxStamina);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetCurrentStamina(float InCurrentStamina);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetStaminaRecoveryPerSecond(float InStaminaRecoveryPerSecond);

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

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetWalkSpeed(float InWalkSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetRunSpeed(float InRunSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetSprintSpeed(float InSprintSpeed);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|MoveSpeed")
	bool HasReachedSprintSpeedRatio(float RequiredRatio) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Defence")
	void SetDefence(float InDefence);

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

private:
	FString MakeStatRowKey() const;
	void TickRecentDamageCooldown(float DeltaTime);
	void TickGroggyRecovery(float DeltaTime);
	void TickRecoverableResourceRecovery(float DeltaTime);
	void BroadcastDeathStarted(EMVDeathReason Reason);
	void RestartRecentDamageCooldown();
	void ResetDamageAccumulation();
	bool TryStartGroggy();
	void BroadcastGroggyEnded();

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
	float StaminaRecoveryPerSecond = 35.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float MaxMP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float CurrentMP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float MPRecoveryPerSecond = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Attack")
	float AttackPower = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Attack")
	float AttackSpeed = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MoveSpeed")
	float WalkSpeed = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MoveSpeed")
	float RunSpeed = 500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MoveSpeed")
	float SprintSpeed = 750.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Defence")
	float Defence = 1.0f;

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
	float RecentDamageCooldownRemaining = 0.0f;
	int32 RecoverableStatRecoveryPauseCount = 0;
	FMVResolvedHitData PendingDeathHitData;
	bool bHasPendingDeathHitData = false;
	bool bHasRecentDamageAccumulation = false;
	bool bIsDead = false;
	bool bIsGroggy = false;

	// Poise
	FTimerHandle PoiseRecoveryTimerHandle;
};
