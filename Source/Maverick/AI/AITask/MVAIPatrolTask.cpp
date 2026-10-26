#include "AI/AITask/MVAIPatrolTask.h"

#include "AIController.h"
#include "AI/Controller/MVAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"

namespace MaverickPatrolTaskPrivate
{
	// 대상 발견은 순찰의 정상 중단, 목적지 도착 성공과 구분
	bool StopPatrolForTarget(FMVAIPatrolTaskInstanceData& InstanceData, AAIController* Controller)
	{
		const AMVAIController* MVController = Cast<AMVAIController>(Controller);
		if (!IsValid(MVController) || !IsValid(MVController->TargetActor))
		{
			return false;
		}

		if (InstanceData.bMoveRequested)
		{
			UE_LOG(LogTemp, Verbose, TEXT("[PatrolTargetStop] Frame=%llu Controller=%s Target=%s"),
				GFrameCounter, *GetNameSafe(Controller), *GetNameSafe(MVController->TargetActor));
			// StopMovement의 완료 알림 전에 순찰 소유 상태 해제
			InstanceData.bMoveRequested = false;
			InstanceData.bPatrolling = false;
			InstanceData.bLastMoveSucceeded = false;
			Controller->StopMovement();
		}
		else
		{
			InstanceData.bPatrolling = false;
			InstanceData.bLastMoveSucceeded = false;
		}
		return true;
	}

	// 이동·경로 대기·일시 정지를 미완료 상태로 분류
	bool IsMoveInProgress(const EPathFollowingStatus::Type Status)
	{
		return Status == EPathFollowingStatus::Moving
			|| Status == EPathFollowingStatus::Waiting
			|| Status == EPathFollowingStatus::Paused;
	}

	// 바인딩된 Pawn 우선 사용, 없으면 실행 소유자의 AIController 또는 Pawn에서 순찰 주체 조회
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

	// 실행 소유자의 AIController 우선 사용, 없으면 순찰 Pawn의 Controller 조회
	AAIController* ResolveController(APawn* Pawn, UObject* ContextOwner)
	{
		if (AAIController* AIController = Cast<AAIController>(ContextOwner))
		{
			return AIController;
		}

		return Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;
	}
}

// 순찰 목적지까지 이동 요청 후 완료 대기 담당, 이동 상태 감시용 Tick 활성화
FMVAIPatrolTask::FMVAIPatrolTask()
{
	bShouldCallTick = true;
}

// 실행 데이터 초기화 후 도달 가능한 무작위 목적지 선택·MoveTo 1회 요청
// 진입 시 유효한 TargetActor 존재 시 이동 요청 없이 Succeeded 반환
// 소유자·내비게이션·이동 요청 오류는 Failed, 요청 수락 시 Running 반환
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
	if (IsValid(Owner) && MaverickPatrolTaskPrivate::StopPatrolForTarget(InstanceData, Controller))
	{
		return EStateTreeRunStatus::Succeeded;
	}

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

// 진행 중에는 새 이동 요청 없이 Running 유지, 이동 종료 후 목적지와의 평면 거리로 성공 판정
// TargetActor 발견 시 이동 중단·Succeeded 반환, bLastMoveSucceeded는 false 유지
// AcceptanceRadius 이내는 Succeeded, 이외 또는 필수 참조 무효는 Failed 반환
EStateTreeRunStatus FMVAIPatrolTask::Tick(
	FStateTreeExecutionContext& Context,
	float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (MaverickPatrolTaskPrivate::StopPatrolForTarget(InstanceData, InstanceData.Controller.Get()))
	{
		return EStateTreeRunStatus::Succeeded;
	}

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

// 미완료 이동 요청이 있으면 Controller 이동 중단, 순찰·요청 상태 초기화
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
