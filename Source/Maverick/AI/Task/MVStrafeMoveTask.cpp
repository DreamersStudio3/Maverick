#include "MVStrafeMoveTask.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FMVStrafeMoveTask::EnterState(FStateTreeExecutionContext& Context,
                                                  const FStateTreeTransitionResult& Transition) const
{
	InstanceDataType& InstanceData = Context.GetInstanceData<InstanceDataType>(*this);

	const AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	
	
	
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}
	APawn* Pawn = Controller->GetPawn();

	if (!Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ElapsedTime = 0.0f;

	FVector ActorLocation = Pawn->GetActorLocation();
	FVector ActorRightVector = Pawn->GetActorRightVector();
	
	auto CheckDirection = [&](FVector Direction)-> bool
	{
		FHitResult HitResult;
		FVector End = ActorLocation + (Direction * InstanceData.TraceDistance);
		return  !Pawn->GetWorld()->LineTraceSingleByChannel(HitResult,ActorLocation,End,ECC_Visibility);
	};
	// 1. 오른쪽 체크
	if (CheckDirection(ActorRightVector))
	{
		InstanceData.ChosenDirection = ActorRightVector;
	}
	// 2. 왼쪽 체크
	else if (CheckDirection(-ActorRightVector))
	{
		InstanceData.ChosenDirection = -ActorRightVector;
	}
	else
	{
		// 양쪽 다 막혔으면 즉시 종료 (또는 그냥 오른쪽으로 밀기)
		return EStateTreeRunStatus::Failed;
	}
 
	return EStateTreeRunStatus::Running;

}

EStateTreeRunStatus FMVStrafeMoveTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	InstanceDataType& InstanceData = Context.GetInstanceData<InstanceDataType>(*this);
	
	AAIController* Controller = Cast<AAIController>(Context.GetOwner());
	
	
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	APawn* Pawn = Controller->GetPawn();
	
	if (!Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	// 1. 이동 입력 (Add Movement Input 사용 시 AnimBP의 Calculate Direction과 잘 맞음)
	Pawn->AddMovementInput(InstanceData.ChosenDirection, 1.0f);
 
	// 2. 시간 업데이트 및 종료 체크
	InstanceData.ElapsedTime += DeltaTime;
	if (InstanceData.ElapsedTime >= InstanceData.MoveDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}
 
	return EStateTreeRunStatus::Running;
}

void FMVStrafeMoveTask::ExitState(FStateTreeExecutionContext& Context,
                                  const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
