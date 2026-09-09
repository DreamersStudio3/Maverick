#pragma once

#include "CoreMinimal.h"
#include "MVStatusEffectEnums.generated.h"

UENUM(BlueprintType)
enum class EMVStatusEffectInstanceScope : uint8
{
	OnePerTarget,
	OnePerSource,
	Independent
};

UENUM(BlueprintType)
enum class EMVStatusEffectDurationPolicy : uint8
{
	Instant,
	Timed,
	Infinite
};

UENUM(BlueprintType)
enum class EMVStatusEffectStackPolicy : uint8
{
	NoStack,
	AddStack,
	Replace
};

UENUM(BlueprintType)
enum class EMVStatusEffectRefreshPolicy : uint8
{
	NoRefresh,
	RefreshDuration,
	ExtendDuration
};

UENUM(BlueprintType)
enum class EMVStatusEffectRemovalReason : uint8
{
	Manual,
	Expired,
	Consumed,
	Dispelled,
	Cleared,
	Invalidated,
	OwnerEnded
};

UENUM(BlueprintType)
enum class EMVStatusEffectApplicationTarget : uint8
{
	EventTarget UMETA(DisplayName = "Event Target"),
	SourceActor UMETA(DisplayName = "Source Actor")
};