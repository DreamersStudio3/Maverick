#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVEnemyDeadTask.generated.h"

class AAIController;
class AMVCharacterBase;
class AMVEnemy;
class UMVDeathComponent;

UENUM(BlueprintType)
enum class EMVEnemyDeadCleanupMode : uint8
{
	None,
	DestroyActor,
	DeactivateActor
};

USTRUCT()
struct FMVEnemyDeadTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	bool bStopAIMovement = true;

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	bool bStopCharacterMovement = true;

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	bool bDisableCharacterMovement = true;

	UPROPERTY(EditAnywhere, Category = "Input|Collision")
	bool bDisableActorCollisionOnEnter = false;

	UPROPERTY(EditAnywhere, Category = "Input|LockOn")
	bool bClearLockOnTargetOnEnter = true;

	UPROPERTY(EditAnywhere, Category = "Input|Death")
	bool bWaitForDeathPresentationFinished = true;

	UPROPERTY(EditAnywhere, Category = "Input|Cleanup")
	EMVEnemyDeadCleanupMode CleanupMode = EMVEnemyDeadCleanupMode::None;

	UPROPERTY(EditAnywhere, Category = "Input|Cleanup", meta = (ClampMin = "0.0"))
	float CleanupDelaySeconds = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Cleanup", meta = (ClampMin = "0.0"))
	float MinimumCleanupDelaySeconds = 3.0f;

	UPROPERTY(Transient)
	TObjectPtr<AMVCharacterBase> Character = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMVDeathComponent> DeathComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> AIController = nullptr;

	bool bEntered = false;
	bool bCleanupDelayStarted = false;
	bool bCleanupApplied = false;
	float CleanupDelayElapsedSeconds = 0.0f;
};

/**
 * Enemy Dead State task.
 *
 * The task does not start death animation. StatComponent and DeathComponent own
 * death detection and presentation; this task clears lock-on targeting the dead
 * enemy, stops AI movement, and waits until DeathComponent reports presentation
 * completion before optional cleanup. Actor destruction is deferred out of the
 * StateTree execution frame.
 */
USTRUCT(meta = (DisplayName = "Enemy Dead Task"))
struct FMVEnemyDeadTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVEnemyDeadTaskInstanceData;

	FMVEnemyDeadTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		float DeltaTime) const override;
};
