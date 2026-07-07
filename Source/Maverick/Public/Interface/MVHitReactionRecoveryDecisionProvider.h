#pragma once

#include "CoreMinimal.h"
#include "Components/MVInputManagerComponent.h"
#include "Tables/MVActionTableTypes.h"
#include "UObject/Interface.h"
#include "MVHitReactionRecoveryDecisionProvider.generated.h"

class AActor;
enum class EMVHitReactionDirection : uint8;

UENUM(BlueprintType)
enum class EMVHitReactionRecoveryDecisionType : uint8
{
	None,
	Getup,
	EscapeDodge
};

USTRUCT(BlueprintType)
struct FMVHitReactionRecoveryDecision
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery")
	EMVHitReactionRecoveryDecisionType Type = EMVHitReactionRecoveryDecisionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|HitReaction|Recovery")
	EMVActionInputDirection EscapeDirection = EMVActionInputDirection::Back;
};

struct FMVHitReactionRecoveryDecisionContext
{
	AActor* Owner = nullptr;
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;
	EMVHitReactionDirection HitReactionDirection;
};

UINTERFACE(MinimalAPI)
class UMVHitReactionRecoveryDecisionProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Provides owner-specific hit reaction recovery choices.
 *
 * HitReactionComponent owns the common recovery execution path. Actors that
 * need AI or character-specific policy implement this interface and return a
 * high-level recovery decision when the recovery window opens.
 */
class MAVERICK_API IMVHitReactionRecoveryDecisionProvider
{
	GENERATED_BODY()

public:
	virtual bool TryChooseHitReactionRecovery(
		const FMVHitReactionRecoveryDecisionContext& Context,
		FMVHitReactionRecoveryDecision& OutDecision)
	{
		return false;
	}
};
