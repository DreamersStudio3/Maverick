#pragma once

#include "CoreMinimal.h"
#include "MVActionTableTypes.generated.h"

UENUM(BlueprintType)
enum class EMVActionDomain : uint8
{
	None UMETA(DisplayName = "None"),
	Attack UMETA(DisplayName = "Attack"),
	Dead UMETA(DisplayName = "Dead"),
	HitReaction UMETA(DisplayName = "Hit Reaction"),
	Dodge UMETA(DisplayName = "Dodge"),
	Skill UMETA(DisplayName = "Skill")
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVActionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action")
	EMVActionDomain Domain = EMVActionDomain::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action")
	FName RowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action")
	FName StartSection = NAME_None;

	bool IsValid() const
	{
		return Domain != EMVActionDomain::None && !RowName.IsNone();
	}

	void Reset()
	{
		Domain = EMVActionDomain::None;
		RowName = NAME_None;
		StartSection = NAME_None;
	}
};

UENUM(BlueprintType)
enum class EMVActionResourceCostType : uint8
{
	None,
	Instant,
	PerSecond,
	OnDemand
};

UENUM(BlueprintType)
enum class EMVActionHitReactionType : uint8
{
	None,
	SmallHit,
	LargeHit,
	KnockDown,
	Airborne,
	Groggy
};

namespace MVActionHitReactions
{
	FORCEINLINE bool IsKnockDownOrAirborne(const EMVActionHitReactionType HitReactionType)
	{
		return HitReactionType == EMVActionHitReactionType::KnockDown
			|| HitReactionType == EMVActionHitReactionType::Airborne;
	}
}
