// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "MVEnemy.generated.h"

/**
 * 
 */
UCLASS()
class MAVERICK_API AMVEnemy : public AMVCharacterBase
{
	GENERATED_BODY()
	
public:
	AMVEnemy();
	
	bool Attack(int EAttackDirection);
	
	
protected:
	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
	
};
