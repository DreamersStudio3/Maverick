#include "AI/Task/MVHitReactionTask.h"

#include "AIController.h"
#include "Character/MVCharacterBase.h"
#include "Components/MVActionComponent.h"
#include "Components/MVHitReaction.h"
#include "StateTreeExecutionContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVHitReactionTask, Log, All);

namespace
{
APawn* HitReactionTaskResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

void HitReactionTaskLogTrace(
	const TCHAR* Stage,
	const FMVHitReactionTaskInstanceData& InstanceData,
	const APawn* Owner = nullptr)
{
	const UMVActionComponent* ActionComponent = InstanceData.ActionComponent.Get();
	const bool bActionRunning = ActionComponent && ActionComponent->IsActionRunning();
	UE_LOG(
		LogMVHitReactionTask,
		Verbose,
		TEXT("EnemyHitReactTrace Frame=%llu Stage=%s Owner=%s Victim=%s HitReactionType=%d bWaitForActionEnd=%s bActionRunning=%s ActiveTable=%s ActiveRow=%s StartedTable=%s StartedRow=%s bCanInterrupt=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Owner),
		*GetNameSafe(InstanceData.HitData.Victim.Get()),
		static_cast<int32>(InstanceData.HitData.HitReactionType),
		InstanceData.bWaitForActionEnd ? TEXT("true") : TEXT("false"),
		bActionRunning ? TEXT("true") : TEXT("false"),
		ActionComponent ? *ActionComponent->GetActiveActionTableName().ToString() : TEXT("<none>"),
		ActionComponent ? *ActionComponent->GetActiveActionRowName().ToString() : TEXT("<none>"),
		*InstanceData.StartedActionTableName.ToString(),
		*InstanceData.StartedActionRowName.ToString(),
		(ActionComponent && ActionComponent->CanInterruptActiveAction()) ? TEXT("true") : TEXT("false"));
}

bool HitReactionTaskIsStartedActionRunning(const FMVHitReactionTaskInstanceData& InstanceData)
{
	if (!InstanceData.ActionComponent || !InstanceData.ActionComponent->IsActionRunning())
	{
		return false;
	}

	if (InstanceData.StartedActionTableName.IsNone() || InstanceData.StartedActionRowName.IsNone())
	{
		return false;
	}

	return InstanceData.ActionComponent->GetActiveActionInstanceId() == InstanceData.StartedActionInstanceId
		&& InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName
		&& InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName;
}

bool HitReactionTaskIsStateTreeHandledType(const EMVActionHitReactionType HitReactionType)
{
	switch (HitReactionType)
	{
	case EMVActionHitReactionType::Flinch:
	case EMVActionHitReactionType::Stagger:
	case EMVActionHitReactionType::Knockback:
	case EMVActionHitReactionType::KnockDown:
	case EMVActionHitReactionType::Airborne:
	case EMVActionHitReactionType::Groggy:
		return true;
	case EMVActionHitReactionType::None:
	default:
		return false;
	}
}

}

FMVHitReactionTask::FMVHitReactionTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVHitReactionTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ActionComponent = nullptr;
	InstanceData.HitReactionComponent = nullptr;
	InstanceData.StartedActionInstanceId = INDEX_NONE;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;
	HitReactionTaskLogTrace(TEXT("Enter"), InstanceData);

	if (InstanceData.HitData.HitReactionType == EMVActionHitReactionType::None)
	{
		HitReactionTaskLogTrace(TEXT("Rejected_NoneType"), InstanceData);
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bRequireStateTreeHandledHitReactionType
		&& !HitReactionTaskIsStateTreeHandledType(InstanceData.HitData.HitReactionType))
	{
		HitReactionTaskLogTrace(TEXT("Rejected_UnsupportedType"), InstanceData);
		return EStateTreeRunStatus::Failed;
	}

	APawn* Owner = HitReactionTaskResolveOwner(Context, InstanceData.Owner);
	if (!Owner || InstanceData.HitData.Victim.Get() != Cast<AMVCharacterBase>(Owner))
	{
		HitReactionTaskLogTrace(TEXT("Rejected_OwnerMismatch"), InstanceData, Owner);
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.HitReactionComponent = Owner->FindComponentByClass<UMVHitReaction>();
	if (!InstanceData.HitReactionComponent)
	{
		HitReactionTaskLogTrace(TEXT("Rejected_MissingHitReactionComponent"), InstanceData, Owner);
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();

	const int32 PreviousActionInstanceId = InstanceData.ActionComponent
		? InstanceData.ActionComponent->GetActiveActionInstanceId() : INDEX_NONE;

	HitReactionTaskLogTrace(TEXT("BeforeHandleHitEvent"), InstanceData, Owner);
	InstanceData.HitReactionComponent->HandleHitEvent(InstanceData.HitData);
	HitReactionTaskLogTrace(TEXT("AfterHandleHitEvent"), InstanceData, Owner);

	if (InstanceData.ActionComponent && InstanceData.ActionComponent->IsActionRunning())
	{
		const FName ActiveActionTableName = InstanceData.ActionComponent->GetActiveActionTableName();
		if (InstanceData.ActionComponent->GetActiveActionInstanceId() != PreviousActionInstanceId)
		{
			InstanceData.StartedActionInstanceId = InstanceData.ActionComponent->GetActiveActionInstanceId();
			InstanceData.StartedActionTableName = ActiveActionTableName;
			InstanceData.StartedActionRowName = InstanceData.ActionComponent->GetActiveActionRowName();
			HitReactionTaskLogTrace(TEXT("StartedHitReactionAction"), InstanceData, Owner);
			return InstanceData.bWaitForActionEnd
				? EStateTreeRunStatus::Running
				: EStateTreeRunStatus::Succeeded;
		}

		HitReactionTaskLogTrace(TEXT("ReactionDidNotStartNewAction"), InstanceData, Owner);
	}

	HitReactionTaskLogTrace(TEXT("Rejected_ReactionNotStarted"), InstanceData, Owner);
	return InstanceData.bFailWhenReactionNotStarted
		? EStateTreeRunStatus::Failed
		: EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FMVHitReactionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (HitReactionTaskIsStartedActionRunning(InstanceData))
	{
		return EStateTreeRunStatus::Running;
	}

	HitReactionTaskLogTrace(TEXT("Completed_ActionEnded"), InstanceData);
	return EStateTreeRunStatus::Succeeded;
}

void FMVHitReactionTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.bCancelReactionOnExit
		&& InstanceData.bWaitForActionEnd
		&& HitReactionTaskIsStartedActionRunning(InstanceData)
		&& (!InstanceData.HitReactionComponent
			|| !InstanceData.HitReactionComponent->IsManagingGroggyAction(InstanceData.StartedActionInstanceId)))
	{
		HitReactionTaskLogTrace(TEXT("Exit_CancelRunningReaction"), InstanceData);
		InstanceData.ActionComponent->CancelActiveAction(InstanceData.ExitCancelBlendOutTime);
	}

	if (InstanceData.bClearHitDataOnExit)
	{
		InstanceData.HitData = FMVResolvedHitData();
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
