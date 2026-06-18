#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVStatComponent.generated.h"

class UMVTableManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnStatValueChanged, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnDead);

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
	FMVOnDead OnDead;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Table")
	void SetStatTableReference(FName InStatTableName, const FString& InStatRowKey);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Table")
	bool LoadStatsFromTable();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|HP")
	void SetMaxHP(float InMaxHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|HP")
	void SetCurrentHP(float InCurrentHP);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetMaxStamina(float InMaxStamina);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetCurrentStamina(float InCurrentStamina);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetStaminaRecoveryPerSecond(float InStaminaRecoveryPerSecond);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	void SetStaminaRecoveryDelay(float InStaminaRecoveryDelay);

	UFUNCTION(BlueprintPure, Category = "Maverick|Stat|Stamina")
	bool HasStamina(float RequiredAmount) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Stamina")
	bool ConsumeStamina(float Amount);

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
	void SetAttackSpeed(float InAttackSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetWalkSpeed(float InWalkSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetRunSpeed(float InRunSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|MoveSpeed")
	void SetSprintSpeed(float InSprintSpeed);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Defence")
	void SetDefence(float InDefence);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetMaxGroggy(float InMaxGroggy);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetCurrentGroggy(float InCurrentGroggy);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetGroggyRecoveryPerSecond(float InGroggyRecoveryPerSecond);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Stat|Groggy")
	void SetGroggyRecoveryDelay(float InGroggyRecoveryDelay);

private:
	bool TryReadFloat(const UMVTableManager* TableManager, const FString& FieldName, float& OutValue) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Table")
	FName StatTableName = TEXT("PlayerStat");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Table")
	FString StatRowKey = TEXT("1");

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|Stamina")
	float StaminaRecoveryDelay = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float MaxMP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float CurrentMP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Stat|MP")
	float MPRecoveryPerSecond = 5.0f;

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
};
