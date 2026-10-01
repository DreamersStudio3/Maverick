#include "AI/AITask/FMVBossExecuteAttackTask.h"

#include "AIController.h"
#include "AI/Controller/MVAIController.h"
#include "Components/MVActionComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "StateTreeExecutionContext.h"

FMVBossExecuteAttackTask::FMVBossExecuteAttackTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVBossExecuteAttackTask::EnterState(FStateTreeExecutionContext& Context,const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ActionComponent = nullptr;
	InstanceData.AnimInstance = nullptr;
	InstanceData.ActiveMontage = nullptr;
	InstanceData.bCustomMontageStopOnExit = true;
	InstanceData.bCompletionLogged = false;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;

	APawn* Owner = InstanceData.Owner;
	if (!Owner)
	{
		if (const AAIController* AIController = Cast<AAIController>(Context.GetOwner()))
		{
			Owner = AIController->GetPawn();
		}
		else
		{
			Owner = Cast<APawn>(Context.GetOwner());
		}
	}

	float TargetDistance = InstanceData.CombatContext.DistanceToTarget;
	bool bHasTarget = InstanceData.CombatContext.bHasTarget;
	if (const AMVAIController* AIController = Cast<AMVAIController>(Context.GetOwner()))
	{
		if (const AActor* TargetActor = AIController->TargetActor)
		{
			bHasTarget = true;
			TargetDistance = FVector::Dist(Owner->GetActorLocation(), TargetActor->GetActorLocation());
		}
	}

	if (!Owner
		|| !bHasTarget
		|| !InstanceData.AttackRow.DataTable
		|| InstanceData.AttackRow.RowName.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossAttack] Start failed: Owner=%s HasTarget=%s Distance=%.1f Range=%.1f Table=%s Row=%s"),
			*GetNameSafe(Owner),
			bHasTarget ? TEXT("true") : TEXT("false"),
			TargetDistance,
			InstanceData.AttackRange,
			*GetNameSafe(InstanceData.AttackRow.DataTable),
			*InstanceData.AttackRow.RowName.ToString());
		return EStateTreeRunStatus::Failed;
	}

	// 공격 범위 밖은 공격 실행 실패가 아니라 Chase 전이를 요청하는 정상 종료
	// Failed를 반환하면 상위 공격 분기가 같은 행들을 즉시 재평가하면서 로그가 반복된다.
	if (TargetDistance > InstanceData.AttackRange)
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("[BossAttack] Out of range: Owner=%s Distance=%.1f Range=%.1f -> Succeeded for Chase transition"),
			*GetNameSafe(Owner),
			TargetDistance,
			InstanceData.AttackRange);
		return EStateTreeRunStatus::Succeeded;
	}

	if (const FMVTutorialBossSkillRow* TutorialSkill = InstanceData.AttackRow.DataTable->FindRow<FMVTutorialBossSkillRow>(
		InstanceData.AttackRow.RowName,
		TEXT("FMVBossExecuteAttackTask"),
		false))
	{
		ACharacter* Character = Cast<ACharacter>(Owner);
		UAnimInstance* AnimInstance = Character && Character->GetMesh()
			? Character->GetMesh()->GetAnimInstance()
			: nullptr;
		UAnimMontage* Montage = TutorialSkill->Montage.LoadSynchronous();
		if (!AnimInstance || !Montage || AnimInstance->Montage_Play(Montage, TutorialSkill->PlayRate) <= 0.0f)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[BossAttack] Start failed: Table=%s Row=%s Montage=%s AnimInstance=%s"),
				*GetNameSafe(InstanceData.AttackRow.DataTable),
				*InstanceData.AttackRow.RowName.ToString(),
				*GetNameSafe(Montage),
				*GetNameSafe(AnimInstance));
			return EStateTreeRunStatus::Failed;
		}

		const FName Section = !InstanceData.StartSection.IsNone()
			? InstanceData.StartSection
			: TutorialSkill->StartSection;
		if (!Section.IsNone() && Montage->IsValidSectionName(Section))
		{
			AnimInstance->Montage_JumpToSection(Section, Montage);
		}

		InstanceData.AnimInstance = AnimInstance;
		InstanceData.ActiveMontage = Montage;
		InstanceData.bCustomMontageStopOnExit = TutorialSkill->bStopOnExit;
	}
	else
	{
		InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
		if (!InstanceData.ActionComponent
			|| !InstanceData.ActionComponent->TryStartActionFromRowHandle(
				InstanceData.AttackRow,
				InstanceData.StartSection))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[BossAttack] Start failed: ActionComponent rejected Table=%s Row=%s"),
				*GetNameSafe(InstanceData.AttackRow.DataTable),
				*InstanceData.AttackRow.RowName.ToString());
			return EStateTreeRunStatus::Failed;
		}
	}

	FString ActionTableName = InstanceData.AttackRow.DataTable->GetName();
	ActionTableName.RemoveFromStart(TEXT("DT_"));
	InstanceData.StartedActionTableName = FName(*ActionTableName);
	InstanceData.StartedActionRowName = InstanceData.AttackRow.RowName;
	UE_LOG(
		LogTemp,
		Display,
		TEXT("[BossAttack] Started: Table=%s Row=%s Montage=%s PlayRate=%.2f"),
		*ActionTableName,
		*InstanceData.AttackRow.RowName.ToString(),
		*GetNameSafe(InstanceData.ActiveMontage),
		InstanceData.ActiveMontage && InstanceData.AnimInstance
			? InstanceData.AnimInstance->Montage_GetPlayRate(InstanceData.ActiveMontage)
			: 1.0f);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVBossExecuteAttackTask::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.ActiveMontage && InstanceData.AnimInstance)
	{
		if (InstanceData.AnimInstance->Montage_IsPlaying(InstanceData.ActiveMontage))
		{
			return EStateTreeRunStatus::Running;
		}

		if (!InstanceData.bCompletionLogged)
		{
			InstanceData.bCompletionLogged = true;
			UE_LOG(
				LogTemp,
				Display,
				TEXT("[BossAttack] Finished: Table=%s Row=%s Montage=%s -> Succeeded"),
				*InstanceData.StartedActionTableName.ToString(),
				*InstanceData.StartedActionRowName.ToString(),
				*GetNameSafe(InstanceData.ActiveMontage));
			Context.RequestTransition(FStateTreeTransitionRequest(FStateTreeStateHandle::Succeeded));
		}
		return EStateTreeRunStatus::Succeeded;
	}

	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	const bool bStartedActionRunning = InstanceData.ActionComponent->IsActionRunning()
		&& InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName
		&& InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName;

	if (bStartedActionRunning)
	{
		return EStateTreeRunStatus::Running;
	}

	if (!InstanceData.bCompletionLogged)
	{
		InstanceData.bCompletionLogged = true;
		UE_LOG(
			LogTemp,
			Display,
			TEXT("[BossAttack] Finished: Table=%s Row=%s -> Succeeded"),
			*InstanceData.StartedActionTableName.ToString(),
			*InstanceData.StartedActionRowName.ToString());
		Context.RequestTransition(FStateTreeTransitionRequest(FStateTreeStateHandle::Succeeded));
	}
	return EStateTreeRunStatus::Succeeded;
}

void FMVBossExecuteAttackTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	UE_LOG(
		LogTemp,
		Display,
		TEXT("[BossAttack] Exit: Table=%s Row=%s RunStatus=%s CurrentState=%s TargetState=%s ChangeType=%d"),
		*InstanceData.StartedActionTableName.ToString(),
		*InstanceData.StartedActionRowName.ToString(),
		*UEnum::GetValueAsString(Transition.CurrentRunStatus),
		*Transition.CurrentState.Describe(),
		*Transition.TargetState.Describe(),
		static_cast<int32>(Transition.ChangeType));

	if (InstanceData.bCustomMontageStopOnExit
		&& InstanceData.ActiveMontage
		&& InstanceData.AnimInstance
		&& InstanceData.AnimInstance->Montage_IsPlaying(InstanceData.ActiveMontage))
	{
		InstanceData.AnimInstance->Montage_Stop(0.1f, InstanceData.ActiveMontage);
	}

	if (InstanceData.ActionComponent
		&& InstanceData.ActionComponent->IsActionRunning()
		&& InstanceData.ActionComponent->GetActiveActionTableName() == InstanceData.StartedActionTableName
		&& InstanceData.ActionComponent->GetActiveActionRowName() == InstanceData.StartedActionRowName)
	{
		InstanceData.ActionComponent->CancelActiveAction(0.1f);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
