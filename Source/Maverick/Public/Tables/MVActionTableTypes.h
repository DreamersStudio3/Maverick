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
	Flinch, // 짧은 약피격. HitReaction row에서 bUseLaunch를 켜면 Ability Launch 값으로 밀린다.
	Stagger, // 서서 크게 휘청이는 중간 경직. Launch를 쓰려면 row에서 bUseLaunch를 켠다.
	Knockback, // 크게 밀려나지만 넘어지지는 않는 피격. Root Motion만 쓸지 Launch를 섞을지는 row가 정한다.
	KnockDown, // 넘어지는 피격. Intro/Fall/Land 흐름에서 Launch를 쓸지는 row의 bUseLaunch가 정한다.
	Airborne, // 공중에 뜨는 피격. VerticalSpeed와 Duration을 넣으면 위로 뜬 뒤 낙하한다.
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
