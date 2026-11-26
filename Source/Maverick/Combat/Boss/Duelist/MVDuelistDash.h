#pragma once

#include "CoreMinimal.h"
class ACharacter;

namespace MVDuelistDash
{
	bool HasPath(const ACharacter* Pawn, const AActor* Target, const FVector& End);
	uint16 Start(ACharacter* Pawn, const FVector& End, float Duration, FName Name);
}
