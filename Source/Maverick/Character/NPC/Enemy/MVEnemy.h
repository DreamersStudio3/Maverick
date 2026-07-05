// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Enum/MVAttackDirection.h"
#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemyWeapon.h"
#include "TimerManager.h"
#include "MVEnemy.generated.h"

class UAnimMontage;
class UMVMainHUDWidget;

/**
 * Enemy character bridge for AI-driven combat.
 *
 * Spawns and attaches the configured weapon actor during BeginPlay, and owns
 * attack montage playback notifications so AI tasks can react to animation
 * completion without reaching into animation state directly. Damage
 * notifications are routed to enemy-specific events so StateTree tasks can
 * decide when to run combat state presentation instead of reaching into
 * lower-level components directly.
 */
UCLASS()
class MAVERICK_API AMVEnemy : public AMVCharacterBase
{
	GENERATED_BODY()
	
public:
	DECLARE_MULTICAST_DELEGATE_ThreeParams(
		FMVEnemyAttackMontageEndedSignature,
		int32 /*AttackInstanceId*/,
		UAnimMontage* /*Montage*/,
		bool /*bInterrupted*/);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVEnemyGroggyStartedSignature);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVEnemyGroggyEndedSignature);

	AMVEnemy();
	virtual void BeginPlay() override;
	bool Attack(EMVAttackDirection AttackDirection);
	bool Attack(EMVAttackDirection AttackDirection, int32& OutAttackInstanceId);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Maverick|Enemy|Combat")
	bool TryHeavyAttack();
	virtual bool TryHeavyAttack_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Maverick|Enemy|Combat")
	bool TrySkillAttack(int32 SkillIndex);
	virtual bool TrySkillAttack_Implementation(int32 SkillIndex);

	void DestroyWeaponActor();

	FMVEnemyAttackMontageEndedSignature OnAttackMontageEnded;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Enemy|Event")
	FMVOnDamagedSignature OnEnemyDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Enemy|Event")
	FMVEnemyGroggyStartedSignature OnEnemyGroggyStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Enemy|Event")
	FMVEnemyGroggyEndedSignature OnEnemyGroggyEnded;
	
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BindDamageHandlers() override;

	void ScheduleBossHUDBindRetry(float DelaySeconds);
	void BindBossHUDToMainHUD();

	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted, int32 AttackInstanceId);

	UFUNCTION()
	void HandleEnemyDamaged(const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleEnemyGroggyStarted();

	UFUNCTION()
	void HandleEnemyGroggyEnded();

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	int32 NextAttackInstanceId = 0;
	
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AMVEnemyWeapon> WeaponClass ;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<AMVEnemyWeapon> WeaponActor;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bUseDualWeapon = true;

	FTimerHandle BossHUDBindRetryTimerHandle;
	TWeakObjectPtr<UMVMainHUDWidget> BoundBossHUD;
	int32 BossHUDBindAttemptsRemaining = 0;
};
