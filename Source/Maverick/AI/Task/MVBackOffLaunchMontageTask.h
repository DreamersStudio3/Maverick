#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "MVBackOffLaunchMontageTask.generated.h"

class AActor;
class APawn;
class UAnimInstance;
class UAnimMontage;

USTRUCT()
struct FMVBackOffLaunchMontageTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Target")
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Animation")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Animation", meta = (ClampMin = "0.0"))
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Animation")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Facing")
	bool bFaceTargetBeforePlay = true;

	UPROPERTY(EditAnywhere, Category = "Input|Launch")
	bool bStopMovementBeforeLaunch = true;

	UPROPERTY(EditAnywhere, Category = "Input|Launch", meta = (ClampMin = "0.0", Units = "cm", ForceUnits = "cm"))
	float LaunchDistance = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Launch", meta = (ClampMin = "0.0", Units = "s"))
	float LaunchDuration = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Input|Launch", meta = (Units = "cm/s", ForceUnits = "cm/s"))
	float LaunchVerticalSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Launch")
	bool bOverrideXYVelocity = true;

	UPROPERTY(EditAnywhere, Category = "Input|Launch")
	bool bOverrideZVelocity = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bWaitForMontageEnd = true;

	UPROPERTY(EditAnywhere, Category = "Input|Task", meta = (ClampMin = "0.0", EditCondition = "bWaitForMontageEnd"))
	float MaxWaitTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Input|Task")
	bool bStopMontageOnExit = false;

	UPROPERTY(EditAnywhere, Category = "Input|Task", meta = (ClampMin = "0.0", EditCondition = "bStopMontageOnExit"))
	float ExitBlendOutTime = 0.1f;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	bool bLaunched = false;

	UPROPERTY(Transient)
	TObjectPtr<APawn> ResolvedOwner = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> AnimInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveMontage = nullptr;

	float ElapsedTime = 0.0f;
	float StartedMontageDuration = 0.0f;
	bool bStartedMontage = false;
};

/**
 * Temporary StateTree task for BackOff presentation.
 *
 * It plays a montage directly on the owner's AnimInstance and uses
 * LaunchCharacter to move the owner away from the target. This intentionally
 * bypasses ActionComponent for one-off prototype animations.
 */
USTRUCT(meta = (DisplayName = "BackOff Launch Montage Task"))
struct FMVBackOffLaunchMontageTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVBackOffLaunchMontageTaskInstanceData;

	FMVBackOffLaunchMontageTask();

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

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
