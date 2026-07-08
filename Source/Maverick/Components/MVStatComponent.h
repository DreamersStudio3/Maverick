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

/**
 * 캐릭터 스탯 값과 회복 정책을 관리하는 컴포넌트.
 *
 * 명시적으로 설정된 CharacterIndexCode와 동일한 CharacterStat row에서 기본 스탯을
 * 로드하고 HP, 스태미너, MP, groggy, 이동/전투 수치의 현재값과 변경 이벤트를 소유한다.
 * `OnDamaged` 구독을 통해 확정된 피해의 HP 차감도 처리한다.
 * NotifyState가 요청한 회복 일시정지와 최근 감소 UI 홀드 이벤트도 이 컴포넌트의 상태로 관리한다.
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

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetGroggyRecoveryDelay(float InGroggyRecoveryDelay);

private:
	FString MakeStatRowKey() const;
	void TickGroggyRecovery(float DeltaTime);
	void TickRecoverableResourceRecovery(float DeltaTime);
	void BroadcastDeathStarted(EMVDeathReason Reason);
	void RestartGroggyRecoveryCooldown();
	bool TryStartGroggy();
	void BroadcastGroggyEnded();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Groggy")
	float GroggyRecoveryDelay = 2.0f;

private:
	float GroggyRecoveryCooldownRemaining = 0.0f;
	int32 RecoverableStatRecoveryPauseCount = 0;
	FMVResolvedHitData PendingDeathHitData;
	bool bHasPendingDeathHitData = false;
	bool bIsDead = false;
	bool bIsGroggy = false;
};
