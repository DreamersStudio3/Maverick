#pragma once

#include "CoreMinimal.h"
#include "MVSkillImplementationState.generated.h"

UENUM(BlueprintType)
enum class EMVSkillImplementationState : uint8
{
	Ready,
	Prototype
};
