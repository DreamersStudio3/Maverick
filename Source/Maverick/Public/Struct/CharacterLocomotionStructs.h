#pragma once

#include "CoreMinimal.h"
#include "CharacterLocomotionStructs.generated.h"

USTRUCT(BlueprintType)
struct FCharacterInputState
{
	GENERATED_BODY()

	FCharacterInputState()
		:WantsToStrafe(false), WantsToAim(false), WantsToWalk(false), WantsToSprint(false)
	{

	}

	UPROPERTY(BlueprintReadWrite)
	uint8 WantsToStrafe : 1;

	UPROPERTY(BlueprintReadWrite)
	uint8 WantsToAim : 1;
	
	UPROPERTY(BlueprintReadWrite)
	uint8 WantsToWalk : 1;
	
	UPROPERTY(BlueprintReadWrite)
	uint8 WantsToSprint : 1;
};