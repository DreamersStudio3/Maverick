#pragma once

#include "CoreMinimal.h"
#include "MVActionTableTypes.generated.h"

namespace MVActionIds
{
	inline constexpr int32 None = 0;
	inline constexpr int32 LightAttack = 10001;
	inline constexpr int32 HeavyAttack = 10002;
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
enum class EMVActionId : uint8
{
	None UMETA(DisplayName = "None"),
	LightAttack UMETA(DisplayName = "Light Attack"),
	HeavyAttack UMETA(DisplayName = "Heavy Attack"),
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
	Airborne
};
