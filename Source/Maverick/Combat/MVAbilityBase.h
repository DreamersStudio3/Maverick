// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Public/Interface/MVAbilityInterface.h"
#include "../Public/Tables/MVSkillDataTableColumn.h"

#include "MVAbilityBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class MAVERICK_API UMVAbilityBase : public UObject, public IMVAbilityInterface
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable)
	void SetOwner(UMVCombatComponent* Owner);
	
	UFUNCTION(BlueprintCallable)
	UMVCombatComponent* GetOwner();
	
	UFUNCTION(BlueprintCallable)
	ACharacter* GetComponentOwner();

	UFUNCTION(BlueprintCallable)
	void InitAbility(FMVSkillDataTableColumn Data);
	
	UFUNCTION(BlueprintCallable)
	bool CheckCooldown();

	UFUNCTION(BlueprintCallable)
	void UpdateLastUsedTime();



	virtual void StartAbility_Implementation() override;
	virtual void EndAbility_Implementation() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UMVCombatComponent> OwnerComponent;
	
	UPROPERTY(BlueprintReadOnly)
	FMVSkillDataTableColumn AbilityData;

	UPROPERTY(BlueprintReadOnly)
	float LastUsedTime = 0.0f;

private:

	
};
