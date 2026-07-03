#pragma once

#include "CoreMinimal.h"
#include "MVActionTableTypes.generated.h"

namespace MVActionIds
{
	inline constexpr int32 None = 0;
	inline constexpr int32 LightAttack = 10001;
	inline constexpr int32 HeavyAttack = 10002;
	inline constexpr int32 ChargeAttack = 10003;
	inline constexpr int32 Skill = 10004;
	inline constexpr int32 Guard = 10010;
	inline constexpr int32 Sprint = 10020;
	inline constexpr int32 Dodge = 10021;
	inline constexpr int32 UseConsumable = 10030;
}

#define ACTIONID_NONE (::MVActionIds::None)
#define ACTIONID_LIGHT_ATTACK (::MVActionIds::LightAttack)
#define ACTIONID_HEAVY_ATTACK (::MVActionIds::HeavyAttack)
#define ACTIONID_GUARD (::MVActionIds::Guard)
#define ACTIONID_SPRINT (::MVActionIds::Sprint)
#define ACTIONID_DODGE (::MVActionIds::Dodge)
#define ACTIONID_USE_CONSUMABLE (::MVActionIds::UseConsumable)

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
enum class EMVActionId : uint8
{
	None UMETA(DisplayName = "None"),
	LightAttack UMETA(DisplayName = "Light Attack"),
	HeavyAttack UMETA(DisplayName = "Heavy Attack"),
	ChargeAttack UMETA(DisplayName = "Charge Attack"),
	Skill UMETA(DisplayName = "Skill"),
	Guard UMETA(DisplayName = "Guard"),
	Sprint UMETA(DisplayName = "Sprint"),
	Dodge UMETA(DisplayName = "Dodge"),
	UseConsumable UMETA(DisplayName = "Use Consumable")
};

namespace MVActionIds
{
	FORCEINLINE int32 ToRawActionId(const EMVActionId ActionId)
	{
		switch (ActionId)
		{
		case EMVActionId::LightAttack:
			return LightAttack;
		case EMVActionId::HeavyAttack:
			return HeavyAttack;
		case EMVActionId::ChargeAttack:
			return ChargeAttack;
		case EMVActionId::Skill:
			return Skill;
		case EMVActionId::Guard:
			return Guard;
		case EMVActionId::Sprint:
			return Sprint;
		case EMVActionId::Dodge:
			return Dodge;
		case EMVActionId::UseConsumable:
			return UseConsumable;
		case EMVActionId::None:
		default:
			return None;
		}
	}
}

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
