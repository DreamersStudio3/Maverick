#include "AI/Task/MVEnemyDeadTask.h"

#include "AIController.h"
#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "Components/MVDeathComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTreeExecutionContext.h"
#include "TimerManager.h"

namespace
{
enum class EMVEnemyDeadTaskCleanupResult : uint8
{
	Finished,
	Waiting
};

APawn* EnemyDeadTaskResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

bool EnemyDeadTaskIsDeathPresentationFinished(const FMVEnemyDeadTaskInstanceData& InstanceData)
{
	if (!InstanceData.bWaitForDeathPresentationFinished)
	{
		return true;
	}

	if (!InstanceData.DeathComponent)
	{
		return true;
	}

	return InstanceData.DeathComponent->GetDeathPresentationPhase() == EMVDeathPresentationPhase::Finished;
}

void EnemyDeadTaskScheduleDestroy(AActor& Owner)
{
	if (Owner.IsActorBeingDestroyed())
	{
		return;
	}

	if (UWorld* World = Owner.GetWorld())
	{
		TWeakObjectPtr<AActor> WeakOwner = &Owner;
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(
			&Owner,
			[WeakOwner]()
			{
				AActor* DeferredOwner = WeakOwner.Get();
				if (IsValid(DeferredOwner) && !DeferredOwner->IsActorBeingDestroyed())
				{
					DeferredOwner->Destroy();
				}
			}));
		return;
	}

	Owner.SetLifeSpan(0.001f);
}

EMVEnemyDeadTaskCleanupResult EnemyDeadTaskApplyCleanup(FMVEnemyDeadTaskInstanceData& InstanceData)
{
	AActor* Owner = InstanceData.Owner.Get();
	if (!Owner)
	{
		return EMVEnemyDeadTaskCleanupResult::Finished;
	}

	switch (InstanceData.CleanupMode)
	{
	case EMVEnemyDeadCleanupMode::DestroyActor:
		if (!InstanceData.bCleanupApplied)
		{
			InstanceData.bCleanupApplied = true;
			EnemyDeadTaskScheduleDestroy(*Owner);
		}
		return EMVEnemyDeadTaskCleanupResult::Waiting;

	case EMVEnemyDeadCleanupMode::DeactivateActor:
		if (InstanceData.bCleanupApplied)
		{
			return EMVEnemyDeadTaskCleanupResult::Finished;
		}

		InstanceData.bCleanupApplied = true;
		if (AMVEnemy* Enemy = Cast<AMVEnemy>(Owner))
		{
			Enemy->DestroyWeaponActor();
		}
		Owner->SetActorEnableCollision(false);
		Owner->SetActorHiddenInGame(true);
		Owner->SetActorTickEnabled(false);
		return EMVEnemyDeadTaskCleanupResult::Finished;

	case EMVEnemyDeadCleanupMode::None:
	default:
		return EMVEnemyDeadTaskCleanupResult::Waiting;
	}
}

}

FMVEnemyDeadTask::FMVEnemyDeadTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVEnemyDeadTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.Character = nullptr;
	InstanceData.DeathComponent = nullptr;
	InstanceData.AIController = nullptr;
	InstanceData.bEntered = false;
	InstanceData.bCleanupDelayStarted = false;
	InstanceData.bCleanupApplied = false;
	InstanceData.CleanupDelayElapsedSeconds = 0.0f;

	APawn* Owner = EnemyDeadTaskResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Owner = Owner;
	InstanceData.Character = Cast<AMVCharacterBase>(Owner);
	InstanceData.DeathComponent = Owner->FindComponentByClass<UMVDeathComponent>();
	InstanceData.AIController = Cast<AAIController>(Owner->GetController());
	if (!InstanceData.AIController)
	{
		InstanceData.AIController = Cast<AAIController>(Context.GetOwner());
	}

	if (InstanceData.bStopAIMovement && InstanceData.AIController)
	{
		InstanceData.AIController->StopMovement();
	}

	if (InstanceData.Character)
	{
		if (UCharacterMovementComponent* MovementComponent = InstanceData.Character->GetCharacterMovement())
		{
			if (InstanceData.bStopCharacterMovement)
			{
				MovementComponent->StopMovementImmediately();
			}

			if (InstanceData.bDisableCharacterMovement)
			{
				MovementComponent->DisableMovement();
			}
		}
	}

	if (InstanceData.bDisableActorCollisionOnEnter)
	{
		Owner->SetActorEnableCollision(false);
	}

	InstanceData.bEntered = true;
	return Tick(Context, 0.0f);
}

EStateTreeRunStatus FMVEnemyDeadTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.bEntered)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!EnemyDeadTaskIsDeathPresentationFinished(InstanceData))
	{
		return EStateTreeRunStatus::Running;
	}

	if (InstanceData.CleanupMode == EMVEnemyDeadCleanupMode::None)
	{
		return EStateTreeRunStatus::Running;
	}

	if (!InstanceData.bCleanupDelayStarted)
	{
		InstanceData.bCleanupDelayStarted = true;
		InstanceData.CleanupDelayElapsedSeconds = 0.0f;
	}
	else
	{
		InstanceData.CleanupDelayElapsedSeconds += FMath::Max(0.0f, DeltaTime);
	}

	if (InstanceData.CleanupDelayElapsedSeconds < InstanceData.CleanupDelaySeconds)
	{
		return EStateTreeRunStatus::Running;
	}

	return EnemyDeadTaskApplyCleanup(InstanceData) == EMVEnemyDeadTaskCleanupResult::Finished
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Running;
}
