// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MVAbilityInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMVAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MAVERICK_API IMVAbilityInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartAbility(int32 AbilityIndex);
	virtual void StartAbility_Implementation(int32 AbilityIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndAbility();
	virtual void EndAbility_Implementation();
};
