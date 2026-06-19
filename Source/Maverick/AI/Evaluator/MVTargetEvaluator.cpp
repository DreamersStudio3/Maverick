#include "MVTargetEvaluator.h"

#include "GameplayTagsManager.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

void FMVTargetEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	InstanceDataType& InstanceData = Context.GetInstanceData<InstanceDataType>(*this);
	
	UWorld* World = Context.GetWorld();
	if (!World)
	{
		return;
	}
	
	InstanceData.Target = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!InstanceData.Target)
	{
		return;
	}
	
	FStateTreeEvaluatorBase::TreeStart(Context);
}

void FMVTargetEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FStateTreeEvaluatorBase::Tick(Context, DeltaTime);
}

void FMVTargetEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	FStateTreeEvaluatorBase::TreeStop(Context);
}
