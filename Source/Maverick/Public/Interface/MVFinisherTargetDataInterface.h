// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MVFinisherTargetDataInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMVFinisherTargetDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MAVERICK_API IMVFinisherTargetDataInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FinisherComponent")
	AActor* GetTargetActor();
};
