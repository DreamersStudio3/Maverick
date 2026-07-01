// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Enum/MVAttackDirection.h"
#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemyWeapon.h"
#include "MVEnemy.generated.h"

class UAnimMontage;

/**
 * Enemy character bridge for AI-driven combat.
 *
 * Spawns and attaches the configured weapon actor during BeginPlay, and owns
 * attack montage playback notifications so AI tasks can react to animation
 * completion without reaching into animation state directly.
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

	AMVEnemy();
	virtual void BeginPlay() override;
	bool Attack(EMVAttackDirection AttackDirection);
	bool Attack(EMVAttackDirection AttackDirection, int32& OutAttackInstanceId);

	FMVEnemyAttackMontageEndedSignature OnAttackMontageEnded;
	
	
protected:
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted, int32 AttackInstanceId);

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	int32 NextAttackInstanceId = 0;
	
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AMVEnemyWeapon> WeaponClass ;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<AMVEnemyWeapon> WeaponActor;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bUseDualWeapon = true;
};
