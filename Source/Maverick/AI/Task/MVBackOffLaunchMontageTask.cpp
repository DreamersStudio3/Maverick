#include "AI/Task/MVBackOffLaunchMontageTask.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "StateTreeExecutionContext.h"

namespace
{
APawn* BackOffLaunchMontageResolveOwner(
	FStateTreeExecutionContext& Context,
	const TObjectPtr<APawn>& BoundOwner)
{
	if (BoundOwner)
	{
		return BoundOwner;
	}

	if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
	{
		return AIController->GetPawn();
	}

	return Cast<APawn>(Context.GetOwner());
}

AActor* BackOffLaunchMontageResolveTarget(APawn& Owner, const TObjectPtr<AActor>& BoundTarget)
{
	if (BoundTarget)
	{
		return BoundTarget;
	}

	UWorld* World = Owner.GetWorld();
	return World ? UGameplayStatics::GetPlayerPawn(World, 0) : nullptr;
}

bool BackOffLaunchMontageBuildAwayDirection(
	const AActor& Owner,
	const AActor& Target,
	FVector& OutAwayDirection)
{
	FVector AwayDirection = Owner.GetActorLocation() - Target.GetActorLocation();
	AwayDirection.Z = 0.0f;

	if (AwayDirection.IsNearlyZero())
	{
		AwayDirection = -Owner.GetActorForwardVector();
		AwayDirection.Z = 0.0f;
	}

	OutAwayDirection = AwayDirection.GetSafeNormal2D();
	return !OutAwayDirection.IsNearlyZero();
}

void BackOffLaunchMontageFaceTarget(APawn& Owner, const FVector& AwayDirection)
{
	const FVector ToTargetDirection = -AwayDirection.GetSafeNormal2D();
	if (!ToTargetDirection.IsNearlyZero())
	{
		Owner.SetActorRotation(ToTargetDirection.Rotation());
	}
}

UAnimInstance* BackOffLaunchMontageGetAnimInstance(ACharacter& Character)
{
	const USkeletalMeshComponent* Mesh = Character.GetMesh();
	return Mesh ? Mesh->GetAnimInstance() : nullptr;
}

bool BackOffLaunchMontagePlay(
	FMVBackOffLaunchMontageTaskInstanceData& InstanceData,
	ACharacter& Character)
{
	if (!InstanceData.Montage)
	{
		return false;
	}

	UAnimInstance* AnimInstance = BackOffLaunchMontageGetAnimInstance(Character);
	if (!AnimInstance)
	{
		return false;
	}

	const float MontageDuration = AnimInstance->Montage_Play(
		InstanceData.Montage,
		FMath::Max(0.0f, InstanceData.PlayRate));
	if (MontageDuration <= 0.0f)
	{
		return false;
	}

	if (!InstanceData.StartSection.IsNone()
		&& InstanceData.Montage->IsValidSectionName(InstanceData.StartSection))
	{
		AnimInstance->Montage_JumpToSection(InstanceData.StartSection, InstanceData.Montage);
	}

	InstanceData.AnimInstance = AnimInstance;
	InstanceData.ActiveMontage = InstanceData.Montage;
	InstanceData.StartedMontageDuration = MontageDuration;
	InstanceData.bStartedMontage = true;
	return true;
}

void BackOffLaunchMontageLaunch(
	FMVBackOffLaunchMontageTaskInstanceData& InstanceData,
	ACharacter& Character,
	const FVector& AwayDirection)
{
	if (InstanceData.bStopMovementBeforeLaunch)
	{
		if (UCharacterMovementComponent* MovementComponent = Character.GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
	}

	const float HorizontalSpeed = InstanceData.LaunchDuration > KINDA_SMALL_NUMBER
		? FMath::Max(0.0f, InstanceData.LaunchDistance) / InstanceData.LaunchDuration
		: 0.0f;

	FVector LaunchVelocity = AwayDirection.GetSafeNormal2D() * HorizontalSpeed;
	LaunchVelocity.Z = InstanceData.LaunchVerticalSpeed;

	if (!LaunchVelocity.IsNearlyZero())
	{
		Character.LaunchCharacter(
			LaunchVelocity,
			InstanceData.bOverrideXYVelocity,
			InstanceData.bOverrideZVelocity);
		InstanceData.bLaunched = true;
	}
}
}

FMVBackOffLaunchMontageTask::FMVBackOffLaunchMontageTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVBackOffLaunchMontageTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ResolvedOwner = nullptr;
	InstanceData.AnimInstance = nullptr;
	InstanceData.ActiveMontage = nullptr;
	InstanceData.ElapsedTime = 0.0f;
	InstanceData.StartedMontageDuration = 0.0f;
	InstanceData.bStartedMontage = false;
	InstanceData.bLaunched = false;

	APawn* Owner = BackOffLaunchMontageResolveOwner(Context, InstanceData.Owner);
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Owner || !Character)
	{
		return EStateTreeRunStatus::Failed;
	}

	AActor* Target = BackOffLaunchMontageResolveTarget(*Owner, InstanceData.Target);
	if (!Target)
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector AwayDirection = FVector::ZeroVector;
	if (!BackOffLaunchMontageBuildAwayDirection(*Owner, *Target, AwayDirection))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bFaceTargetBeforePlay)
	{
		BackOffLaunchMontageFaceTarget(*Owner, AwayDirection);
	}

	if (!BackOffLaunchMontagePlay(InstanceData, *Character))
	{
		return EStateTreeRunStatus::Failed;
	}

	BackOffLaunchMontageLaunch(InstanceData, *Character, AwayDirection);
	InstanceData.ResolvedOwner = Owner;

	return InstanceData.bWaitForMontageEnd ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FMVBackOffLaunchMontageTask::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.bWaitForMontageEnd)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.ElapsedTime += DeltaTime;
	if (InstanceData.MaxWaitTime > 0.0f && InstanceData.ElapsedTime >= InstanceData.MaxWaitTime)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (!InstanceData.bStartedMontage || !InstanceData.AnimInstance || !InstanceData.ActiveMontage)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (!InstanceData.AnimInstance->Montage_IsPlaying(InstanceData.ActiveMontage))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.MaxWaitTime <= 0.0f
		&& InstanceData.StartedMontageDuration > 0.0f
		&& InstanceData.ElapsedTime > InstanceData.StartedMontageDuration + 0.25f)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FMVBackOffLaunchMontageTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.bStopMontageOnExit
		&& InstanceData.bStartedMontage
		&& InstanceData.AnimInstance
		&& InstanceData.ActiveMontage
		&& InstanceData.AnimInstance->Montage_IsPlaying(InstanceData.ActiveMontage))
	{
		InstanceData.AnimInstance->Montage_Stop(
			FMath::Max(0.0f, InstanceData.ExitBlendOutTime),
			InstanceData.ActiveMontage);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
