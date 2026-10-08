#include "AI/AITask/FMVBossExecuteAttackTask.h"

#include "AIController.h"
#include "AI/Controller/MVAIController.h"
#include "Components/MVActionComponent.h"
#include "Components/MVCombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "StateTreeExecutionContext.h"

// 지정된 공격 행 실행·종료 감시 담당, 공격 종류 선택과 타격 판정은 각각 선택 Task·Ability 책임
// 공격 종료 감시용 Tick 활성화
FMVBossExecuteAttackTask::FMVBossExecuteAttackTask()
{
	bShouldCallTick = true;
}

// 소유자·대상·공격 행 검증 후 공격 1회 시작, 범위 밖은 공격 생략 후 Succeeded 반환
// CombatAttackRow 연결 시 CombatComponent의 Ability 경로, 미연결 선택 행은 몽타주 직접 재생
// 전투 행 직접 연결도 CombatComponent 경유, 그 외 행은 ActionComponent 경로 사용
EStateTreeRunStatus FMVBossExecuteAttackTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ActionComponent = nullptr;
	InstanceData.AnimInstance = nullptr;
	InstanceData.ActiveMontage = nullptr;
	InstanceData.bCustomMontageStopOnExit = true;
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
		if (const AActor* TargetActor = AIController->TargetActor; Owner && TargetActor)
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
		return EStateTreeRunStatus::Failed;
	}

	// 공격 범위 밖은 공격 실행 실패가 아니라 Chase 전이를 요청하는 정상 종료
	// Failed를 반환하면 상위 공격 분기가 같은 행들을 즉시 재평가하면서 로그가 반복된다.
	if (TargetDistance > InstanceData.AttackRange)
	{
		// 완료 판정 제외 Task도 공격 트리를 종료하도록 명시적 성공 전이 요청
		Context.RequestTransition(FStateTreeTransitionRequest(FStateTreeStateHandle::Succeeded));
		return EStateTreeRunStatus::Succeeded;
	}

	FDataTableRowHandle ExecutionRow = InstanceData.AttackRow;
	const FMVTutorialBossSkillRow* TutorialSkill = InstanceData.AttackRow.DataTable->GetRowStruct() == FMVTutorialBossSkillRow::StaticStruct()
		? InstanceData.AttackRow.DataTable->FindRow<FMVTutorialBossSkillRow>(
		InstanceData.AttackRow.RowName,
		TEXT("FMVBossExecuteAttackTask"),
		false) : nullptr;
	if (TutorialSkill && TutorialSkill->CombatAttackRow.DataTable)
	{
		ExecutionRow = TutorialSkill->CombatAttackRow;
		UMVCombatComponent* Combat = Owner->FindComponentByClass<UMVCombatComponent>();
		InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
		const FName Section = InstanceData.StartSection.IsNone() ? TutorialSkill->StartSection : InstanceData.StartSection;
		if (!Combat || !InstanceData.ActionComponent || !Combat->TryStartCombatActionFromRowHandle(ExecutionRow, Section))
		{
			return EStateTreeRunStatus::Failed;
		}
	}
	else if (TutorialSkill)
	{
		ACharacter* Character = Cast<ACharacter>(Owner);
		UAnimInstance* AnimInstance = Character && Character->GetMesh()
			? Character->GetMesh()->GetAnimInstance()
			: nullptr;
		UAnimMontage* Montage = TutorialSkill->Montage.LoadSynchronous();
		if (!AnimInstance || !Montage || AnimInstance->Montage_Play(Montage, TutorialSkill->PlayRate) <= 0.0f)
		{
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
	else if (InstanceData.AttackRow.DataTable->GetRowStruct()->IsChildOf(FMVSkillDataTableColumn::StaticStruct()))
	{
		UMVCombatComponent* Combat = Owner->FindComponentByClass<UMVCombatComponent>();
		InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
		if (!Combat || !InstanceData.ActionComponent
			|| !Combat->TryStartCombatActionFromRowHandle(ExecutionRow, InstanceData.StartSection))
		{
			return EStateTreeRunStatus::Failed;
		}
	}
	else
	{
		InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
		if (!InstanceData.ActionComponent
			|| !InstanceData.ActionComponent->TryStartActionFromRowHandle(
				InstanceData.AttackRow,
				InstanceData.StartSection))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	FString ActionTableName = ExecutionRow.DataTable
		? ExecutionRow.DataTable->GetName()
		: InstanceData.AttackRow.DataTable->GetName();
	ActionTableName.RemoveFromStart(TEXT("DT_"));
	InstanceData.StartedActionTableName = FName(*ActionTableName);
	InstanceData.StartedActionRowName = ExecutionRow.RowName;
	return EStateTreeRunStatus::Running;
}

// 직접 재생한 몽타주 또는 시작한 테이블·행의 액션이 진행 중이면 Running 유지
// 감시 대상 종료 시 성공 전이 요청·Succeeded 반환, 액션 경로의 컴포넌트 누락은 Failed
// 완료 후 Chase·메인 트리 복귀 목적지는 StateTree 전이 설정 책임
EStateTreeRunStatus FMVBossExecuteAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.ActiveMontage && InstanceData.AnimInstance)
	{
		if (InstanceData.AnimInstance->Montage_IsPlaying(InstanceData.ActiveMontage))
		{
			return EStateTreeRunStatus::Running;
		}

		Context.RequestTransition(FStateTreeTransitionRequest(FStateTreeStateHandle::Succeeded));
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

	Context.RequestTransition(FStateTreeTransitionRequest(FStateTreeStateHandle::Succeeded));
	return EStateTreeRunStatus::Succeeded;
}

// 직접 재생 몽타주는 bStopOnExit 설정에 따라 중단, 액션은 시작한 테이블·행과 일치할 때만 취소
// 액션 취소에 따른 Ability 종료·타격 타이머 정리는 전투 컴포넌트와 Ability 경로에 위임
void FMVBossExecuteAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
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
