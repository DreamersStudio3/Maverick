#pragma once

#include "CoreMinimal.h"
#include "AI/Enum/MVBossCombatArea.h"
#include "Engine/DataTable.h"
#include "MVAICombatTypes.generated.h"

UENUM(BlueprintType)
enum class EMVAICombatActionRole : uint8
{
	None,
	CounterAttack,
	SprintAttack,
	AirborneChargeAttack,
	BackAttack,
	RangeAttack,
	GapCloseAttack,
	SpecialAttack,
	BasicAttack
};

UENUM(BlueprintType)
enum class EMVAICombatDecisionState : uint8
{
	Dead,
	CounterAttack,
	SprintAttack,
	AirborneChargeAttack,
	SkillAttack,
	BasicAttack,
	MoveToTarget,
	Strafe,
	Idle
};

UENUM(BlueprintType)
enum class EMVAICombatAttackSelectionMode : uint8
{
	SkillAttack,
	BasicAttack
};

USTRUCT(BlueprintType)
struct FMVAICombatActionCandidate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	int32 ActionId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName ActionTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName CooldownActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	EMVAICombatActionRole Role = EMVAICombatActionRole::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float MaxDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float MaxAbsAngle = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bRequiresLineOfSight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bRequiresForwardPathClear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bRequiresBackwardPathClear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bRequiresTargetCanBeAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bRequiresTargetNotAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bRequiresAirbornePattern = false;
};

USTRUCT(BlueprintType)
struct FMVAICombatContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	bool bHasTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	float DistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	float AngleToTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	EMVBossCombatArea CurrentArea = EMVBossCombatArea::OutsideArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	bool bHasLineOfSight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	bool bActionRunning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	TArray<FName> ReadyActionIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName LastAttackTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	bool bAttackCadenceReady = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	int32 CurrentPhase = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Counter")
	bool bCounterWindow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	bool bSprintPathClear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	bool bAirborneChargePathClear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	bool bTargetCanBeAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	bool bTargetIsAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
	bool bShouldUseAirborneCharge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	bool bNeedAttackAngle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	bool bNeedClearAttackPath = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	bool bStrafePathClear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life")
	bool bIsDead = false;
};

namespace MVAICombat
{
	FORCEINLINE FName MakeCooldownActionId(const FMVAICombatActionCandidate& Candidate)
	{
		if (!Candidate.CooldownActionId.IsNone())
		{
			return Candidate.CooldownActionId;
		}

		if (!Candidate.ActionRow.RowName.IsNone())
		{
			return Candidate.ActionRow.RowName;
		}

		return Candidate.ActionId > 0 ? FName(*FString::FromInt(Candidate.ActionId)) : NAME_None;
	}

	FORCEINLINE FName MakeActionTag(const FMVAICombatActionCandidate& Candidate)
	{
		if (!Candidate.ActionTag.IsNone())
		{
			return Candidate.ActionTag;
		}

		if (!Candidate.ActionRow.RowName.IsNone())
		{
			return Candidate.ActionRow.RowName;
		}

		return Candidate.ActionId > 0 ? FName(*FString::FromInt(Candidate.ActionId)) : NAME_None;
	}

	FORCEINLINE bool IsActionReady(const FMVAICombatContext& Context, const FName ActionId)
	{
		return !ActionId.IsNone() && Context.ReadyActionIds.Contains(ActionId);
	}

	FORCEINLINE bool HasExecutableActionRow(const FMVAICombatActionCandidate& Candidate)
	{
		return Candidate.ActionRow.DataTable && !Candidate.ActionRow.RowName.IsNone();
	}

	FORCEINLINE bool IsDistanceInRange(const FMVAICombatContext& Context, const float MinDistance, const float MaxDistance)
	{
		if (Context.DistanceToTarget < MinDistance)
		{
			return false;
		}

		return MaxDistance <= 0.0f || Context.DistanceToTarget <= MaxDistance;
	}
}
