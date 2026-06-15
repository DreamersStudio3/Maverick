#include "MVBossLookAtTargetTask.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialExpressionLocalPosition.h"

EStateTreeRunStatus FMVBossLookAtTargetTask::EnterState(FStateTreeExecutionContext& Context,
                                                        const FStateTreeTransitionResult& Transition) const
{
	FInstanceData& InstanceData = Context.GetInstanceData<FInstanceData>(*this); 
	InstanceData.Owner = Cast<AActor>(Context.GetOwner());
	if (!InstanceData.Owner)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	UWorld* World = InstanceData.Owner->GetWorld();
	if (!World)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	InstanceData.Target = UGameplayStatics::GetPlayerPawn(World, 0);
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVBossLookAtTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	
	FInstanceData& InstanceData = Context.GetInstanceData<FInstanceData>(*this);
	
	FVector OwnerLocation = InstanceData.Owner.Get()->GetActorLocation();
	FVector TargetLocation = InstanceData.Target->GetActorLocation();
	
	FVector TargetDirection = TargetLocation - OwnerLocation;
	TargetDirection.Z = 0.0f;
	
	if (TargetDirection.IsNearlyZero())
	{
		return EStateTreeRunStatus::Running;
	}
	
	const FRotator TargetRotation = TargetDirection.Rotation();
	const FRotator CurrentRotation = InstanceData.Owner->GetActorRotation();
	
	const FRotator NewRotator = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		5.0
		);
	
	InstanceData.Owner->SetActorRotation(NewRotator);
	
	return EStateTreeRunStatus::Running;
}

void FMVBossLookAtTargetTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
