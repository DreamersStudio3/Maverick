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
	Flinch, // 짧은 약피격, Launch값 설정 필요
	Stagger, // 서서 크게 휘청임 / 중간 경직, Launch값 설정 필요
	Knockback, // 크게 밀려남, 넘어지지는 않음, Launch 적용안됨(Root Motion)
	KnockDown, // 넘어짐, Launch 적용안됨(Root Motion)
	Airborne, // 공중에 뜸, Launch값 설정 필요
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
