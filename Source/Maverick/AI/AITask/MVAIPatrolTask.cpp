#include "AI/AITask/MVAIPatrolTask.h"

#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"

namespace MaverickPatrolTaskPrivate
{
	bool IsMoveInProgress(const EPathFollowingStatus::Type Status)
	{
		return Status == EPathFollowingStatus::Moving
			|| Status == EPathFollowingStatus::Waiting
			|| Status == EPathFollowingStatus::Paused;
	}

	APawn* ResolveOwner(FStateTreeExecutionContext& Context, APawn* BoundOwner)
	{
		if (IsValid(BoundOwner))
		{
			return BoundOwner;
		}

		if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
		{
			return AIController->GetPawn();
		}

		return Cast<APawn>(Context.GetOwner());
	}

	AAIController* ResolveController(APawn* Pawn, UObject* ContextOwner)
	{
		if (AAIController* AIController = Cast<AAIController>(ContextOwner))
		{
			return AIController;
		}

		return Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;
	}
}

FMVAIPatrolTask::FMVAIPatrolTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVAIPatrolTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.bPatrolling = false;
	InstanceData.bLastMoveSucceeded = false;
	InstanceData.bMoveRequested = false;
	InstanceData.Destination = FVector::ZeroVector;
	InstanceData.Controller = nullptr;

	APawn* Owner = MaverickPatrolTaskPrivate::ResolveOwner(Context, InstanceData.Owner);
	AAIController* Controller = MaverickPatrolTaskPrivate::ResolveController(Owner, Context.GetOwner());
	if (!IsValid(Owner) || !IsValid(Controller) || !IsValid(Controller->GetPathFollowingComponent()))
	{
		return EStateTreeRunStatus::Failed;
	}

	FNavLocation RandomLocation;
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Context.GetWorld());
	if (!NavigationSystem
		|| !NavigationSystem->GetRandomReachablePointInRadius(
			Owner->GetActorLocation(),
			InstanceData.Radius,
			RandomLocation,
			nullptr))
	{
		return EStateTreeRunStatus::Failed;
	}

	const FAIMoveRequest MoveRequest(RandomLocation.Location);
	const FPathFollowingRequestResult MoveResult = Controller->MoveTo(MoveRequest);
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Controller = Controller;
	InstanceData.Owner = Owner;
	InstanceData.Destination = RandomLocation.Location;
	InstanceData.bPatrolling = true;
	InstanceData.bMoveRequested = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVAIPatrolTask::Tick(
	FStateTreeExecutionContext& Context,
	float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.bMoveRequested || !IsValid(InstanceData.Controller))
	{
		return EStateTreeRunStatus::Failed;
	}

	const APawn* Owner = InstanceData.Owner;
	const AAIController* Controller = InstanceData.Controller;
	const UPathFollowingComponent* PathFollowing = Controller->GetPathFollowingComponent();
	if (!IsValid(Owner) || !IsValid(PathFollowing))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (MaverickPatrolTaskPrivate::IsMoveInProgress(PathFollowing->GetStatus()))
	{
		return EStateTreeRunStatus::Running;
	}

	const float DistanceToDestination = FVector::Dist2D(
		Owner->GetActorLocation(),
		InstanceData.Destination);
	InstanceData.bLastMoveSucceeded = DistanceToDestination <= FMath::Max(0.0f, InstanceData.AcceptanceRadius);
	InstanceData.bPatrolling = false;
	InstanceData.bMoveRequested = false;
	return InstanceData.bLastMoveSucceeded
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Failed;
}

void FMVAIPatrolTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.bMoveRequested && IsValid(InstanceData.Controller))
	{
		InstanceData.Controller->StopMovement();
	}

	InstanceData.bPatrolling = false;
	InstanceData.bMoveRequested = false;
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
