#pragma once

#include "CoreMinimal.h"
#include "Components/MVInputManagerComponent.h"
#include "MVAIDodgeTypes.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct MAVERICK_API FMVAIDodgeRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	EMVActionInputDirection Direction = EMVActionInputDirection::Back;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	TObjectPtr<AActor> ThreatActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	FVector ThreatLocation = FVector::ZeroVector;
};
