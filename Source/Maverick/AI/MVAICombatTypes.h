#pragma once

#include "CoreMinimal.h"
#include "AI/Enum/MVBossCombatArea.h"
#include "Engine/DataTable.h"
#include "Tables/MVActionTableTypes.h"
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

USTRUCT(BlueprintType)
struct FMVAttackActionRowHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FDataTableRowHandle ActionRow;

	bool IsValid() const
	{
		return ActionRow.DataTable && !ActionRow.RowName.IsNone();
	}

	void Reset()
	{
		ActionRow.DataTable = nullptr;
		ActionRow.RowName = NAME_None;
	}
};

USTRUCT(BlueprintType)
struct FMVAICombatActionMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName CooldownActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	EMVAICombatActionRole Role = EMVAICombatActionRole::None;
};

USTRUCT(BlueprintType)
struct FMVAICombatActionCondition
{
	GENERATED_BODY()

	FMVAICombatActionCondition()
	{
		ActionRequest.Domain = EMVActionDomain::Attack;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FMVActionRequest ActionRequest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FMVAICombatActionMetadata Metadata;

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
struct FMVAICombatResolvedAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName CooldownActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	EMVAICombatActionRole Role = EMVAICombatActionRole::None;
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
	FORCEINLINE FName MakeCooldownActionId(
		const FMVAICombatActionMetadata& Metadata,
		const FMVActionRequest& ActionRequest)
	{
		if (!Metadata.CooldownActionId.IsNone())
		{
			return Metadata.CooldownActionId;
		}

		return ActionRequest.RowName;
	}

	FORCEINLINE FName MakeCooldownActionId(const FMVAICombatResolvedAction& ResolvedAction)
	{
		if (!ResolvedAction.CooldownActionId.IsNone())
		{
			return ResolvedAction.CooldownActionId;
		}

		if (!ResolvedAction.ActionRow.RowName.IsNone())
		{
			return ResolvedAction.ActionRow.RowName;
		}

		return NAME_None;
	}

	FORCEINLINE FName MakeActionTag(const FMVAICombatResolvedAction& ResolvedAction)
	{
		if (!ResolvedAction.ActionRow.RowName.IsNone())
		{
			return ResolvedAction.ActionRow.RowName;
		}

		return NAME_None;
	}

	FORCEINLINE bool IsActionReady(const FMVAICombatContext& Context, const FName ActionId)
	{
		return !ActionId.IsNone() && Context.ReadyActionIds.Contains(ActionId);
	}

	FORCEINLINE bool HasExecutableActionRow(const FMVAICombatResolvedAction& ResolvedAction)
	{
		return ResolvedAction.ActionRow.DataTable && !ResolvedAction.ActionRow.RowName.IsNone();
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
