// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Enum/MVAttackDirection.h"
#include "Character/MVCharacterBase.h"
#include "Interface/MVHitReactionRecoveryDecisionProvider.h"
#include "TimerManager.h"
#include "MVEnemy.generated.h"

class UAnimMontage;
class UMVMainHUDWidget;
struct FMVAIDodgeRequest;

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

	// 실행 중인 AI StateTree 하나에 위협 정보 전달, 회피 판단은 StateTree 소유
	bool ReceiveAttackNotice(const FMVAIDodgeRequest& Notice);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Maverick|Enemy|Combat")
	bool TryHeavyAttack(int32 ActionIndex = 0, FName StartSection = NAME_None);
	virtual bool TryHeavyAttack_Implementation(int32 ActionIndex, FName StartSection);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Maverick|Enemy|Combat")
	bool TrySkillAttack(int32 SkillIndex, FName StartSection = NAME_None);
	virtual bool TrySkillAttack_Implementation(int32 SkillIndex, FName StartSection);

	void HideBoundBossHUD();

	void ResetForFieldTransition();

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
	void RestartStateTreeLogicForFieldTransition();

	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted, int32 AttackInstanceId);

	UFUNCTION()
	void HandleEnemyDamaged(const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleEnemyGroggyStarted();

	UFUNCTION()
	void HandleEnemyGroggyEnded();

	AActor* ResolveHitReactionRecoveryTarget() const;
	EMVActionInputDirection ResolveEscapeDirectionAwayFromTarget(const AActor& Target) const;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	int32 NextAttackInstanceId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Enemy|HitReaction|Recovery")
	bool bUseAirborneRecoveryDecision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Enemy|HitReaction|Recovery", meta = (ClampMin = "0.0", Units = "cm"))
	float AirborneEscapeDodgeDistance = 500.0f;

	FTimerHandle BossHUDBindRetryTimerHandle;
	TWeakObjectPtr<UMVMainHUDWidget> BoundBossHUD;
	int32 BossHUDBindAttemptsRemaining = 0;
};
