#pragma once

#include "CoreMinimal.h"
#include "Components/MVInputManagerComponent.h"
#include "GameplayTagContainer.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (Categories = "Action.Combat"))
	FGameplayTag ThreatActionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge", meta = (ClampMin = "0.0"))
	float DistanceToThreat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|AI|Dodge")
	float AngleToThreat = 0.0f;
};
