#include "AI/Task/MVExecuteAttackTask.h"

#include "AI/MVActionCooldownComponent.h"
#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "AIController.h"
#include "Chooser.h"
#include "Components/MVActionComponent.h"
#include "Engine/DataTable.h"
#include "Struct/MVCombatActionTableInput.h"
#include "StateTreeAsyncExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "Tags/MVGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVExecuteAttackTask, Log, All);

namespace
{
APawn* ExecuteAttackResolveOwner(FStateTreeExecutionContext& Context, const TObjectPtr<APawn>& BoundOwner)
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

FName ExecuteAttackActionTableNameFromDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		return NAME_None;
	}

	FString TableName = DataTable->GetName();
	TableName.RemoveFromStart(TEXT("DT_"));
	return FName(*TableName);
}

FGameplayTag ExecuteAttackResolveActionTypeTag(const FMVActionRequest& ActionRequest)
{
	const FString RowNameString = ActionRequest.RowName.ToString();
	if (RowNameString.StartsWith(TEXT("Skill"), ESearchCase::IgnoreCase))
	{
		return MVGameplayTags::Action_Combat_Skill;
	}
	if (RowNameString.Contains(TEXT("Dodge"), ESearchCase::IgnoreCase))
	{
		return RowNameString.Contains(TEXT("Light"), ESearchCase::IgnoreCase)
			? MVGameplayTags::Action_Combat_DodgeLightAttack
			: MVGameplayTags::Action_Combat_DodgeHeavyAttack;
	}
	if (RowNameString.Contains(TEXT("Sprint"), ESearchCase::IgnoreCase))
	{
		return RowNameString.Contains(TEXT("Light"), ESearchCase::IgnoreCase)
			? MVGameplayTags::Action_Combat_SprintLightAttack
			: MVGameplayTags::Action_Combat_SprintHeavyAttack;
	}
	if (RowNameString.Contains(TEXT("Charge"), ESearchCase::IgnoreCase))
	{
		return MVGameplayTags::Action_Combat_ChargeAttack;
	}
	if (RowNameString.Contains(TEXT("Heavy"), ESearchCase::IgnoreCase))
	{
		return MVGameplayTags::Action_Combat_HeavyAttack;
	}
	if (RowNameString.Contains(TEXT("Light"), ESearchCase::IgnoreCase))
	{
		return MVGameplayTags::Action_Combat_LightAttack;
	}

	return FGameplayTag();
}

FMVCombatActionTableInput ExecuteAttackMakeCombatChooserInput(
	const UObject& OwnerObject,
	const FMVActionRequest& ActionRequest)
{
	FMVCombatActionTableInput ChooserInput;
	if (const AMVCharacterBase* Character = Cast<AMVCharacterBase>(&OwnerObject))
	{
		ChooserInput.CurrentWeaponStyle = Character->GetEquippedStyle();
	}
	ChooserInput.SetActionType(ExecuteAttackResolveActionTypeTag(ActionRequest));
	return ChooserInput;
}

bool ExecuteAttackEvaluateChooserActionRowHandle(
	UObject& OwnerObject,
	const FSoftObjectPath& AttackChooserTable,
	FMVActionRequest& ActionRequest,
	FMVAttackActionRowHandle& ChooserAttackActionRowHandle,
	FMVAttackActionRowHandle& OutActionRowHandle)
{
	OutActionRowHandle.Reset();
	ChooserAttackActionRowHandle.Reset();

	if (!AttackChooserTable.IsValid())
	{
		UE_LOG(LogMVExecuteAttackTask, Warning, TEXT("Attack chooser path is invalid."));
		return false;
	}

	UChooserTable* ChooserTable = Cast<UChooserTable>(AttackChooserTable.TryLoad());
	if (!ChooserTable)
	{
		UE_LOG(
			LogMVExecuteAttackTask,
			Warning,
			TEXT("Attack chooser failed to load. Path=%s."),
			*AttackChooserTable.ToString());
		return false;
	}

	FMVCombatActionTableInput CombatChooserInput = ExecuteAttackMakeCombatChooserInput(OwnerObject, ActionRequest);
	FGameplayTag ActionTypeTag = CombatChooserInput.ActionType;
	FGameplayTagContainer ActionTypeTags = CombatChooserInput.ActionTypeTags;

	FChooserEvaluationContext ChooserContext;
	ChooserContext.AddObjectParam(&OwnerObject);
	ChooserContext.AddStructParam(ActionRequest);
	ChooserContext.AddStructParam(CombatChooserInput);
	ChooserContext.AddStructParam(ChooserAttackActionRowHandle);
	ChooserContext.AddStructParam(ActionTypeTag);
	ChooserContext.AddStructParam(ActionTypeTags);

	TSoftObjectPtr<UObject> SelectedObject;
	UChooserTable::EvaluateChooser(
		ChooserContext,
		ChooserTable,
		FObjectChooserBase::FObjectChooserSoftObjectIteratorCallback::CreateLambda(
			[&SelectedObject](const TSoftObjectPtr<UObject>& InResult)
			{
				SelectedObject = InResult;
				return FObjectChooserBase::EIteratorStatus::Stop;
			}));

	if (ChooserAttackActionRowHandle.ActionRow.DataTable)
	{
		OutActionRowHandle = ChooserAttackActionRowHandle;
		OutActionRowHandle.ActionRow.RowName = ActionRequest.RowName;
		return OutActionRowHandle.IsValid();
	}

	UObject* ResolvedObject = SelectedObject.IsValid()
		? SelectedObject.Get()
		: SelectedObject.LoadSynchronous();
	UDataTable* SelectedDataTable = Cast<UDataTable>(ResolvedObject);
	if (!SelectedDataTable)
	{
		UE_LOG(
			LogMVExecuteAttackTask,
			Warning,
			TEXT("Attack chooser did not return FMVAttackActionRowHandle or UDataTable fallback. SelectedObject=%s."),
			*GetNameSafe(ResolvedObject));
		return false;
	}

	OutActionRowHandle.ActionRow.DataTable = SelectedDataTable;
	OutActionRowHandle.ActionRow.RowName = ActionRequest.RowName;
	return OutActionRowHandle.IsValid();
}

FMVActionRequest ExecuteAttackResolveActionRequest(
	const FMVActionRequest& ActionRequest)
{
	FMVActionRequest ResolvedActionRequest = ActionRequest;
	if (ResolvedActionRequest.Domain == EMVActionDomain::None)
	{
		ResolvedActionRequest.Domain = EMVActionDomain::Attack;
	}

	return ResolvedActionRequest;
}

bool ExecuteAttackResolveActionCandidate(
	APawn& Owner,
	const FMVAICombatActionMetadata& AttackMetadata,
	FMVActionRequest& ActionRequest,
	const FSoftObjectPath& AttackChooserTable,
	FMVAttackActionRowHandle& ChooserAttackActionRowHandle,
	FMVAICombatResolvedAction& OutAttack)
{
	OutAttack = FMVAICombatResolvedAction();
	OutAttack.StartSection = ActionRequest.StartSection;
	OutAttack.CooldownActionId = AttackMetadata.CooldownActionId;
	OutAttack.Role = AttackMetadata.Role;

	if (ActionRequest.Domain != EMVActionDomain::Attack)
	{
		UE_LOG(
			LogMVExecuteAttackTask,
			Warning,
			TEXT("Execute Attack Task received non-attack action domain. Domain=%d RowName=%s."),
			static_cast<int32>(ActionRequest.Domain),
			*ActionRequest.RowName.ToString());
		return false;
	}

	if (ActionRequest.RowName.IsNone())
	{
		UE_LOG(LogMVExecuteAttackTask, Warning, TEXT("Execute Attack Task requires ActionRequest.RowName."));
		return false;
	}

	FMVAttackActionRowHandle ResolvedActionRowHandle;
	if (!ExecuteAttackEvaluateChooserActionRowHandle(
		Owner,
		AttackChooserTable,
		ActionRequest,
		ChooserAttackActionRowHandle,
		ResolvedActionRowHandle))
	{
		return false;
	}

	OutAttack.ActionRow = ResolvedActionRowHandle.ActionRow;
	return MVAICombat::HasExecutableActionRow(OutAttack);
}

bool ExecuteAttackTryStartAction(
	UMVActionComponent& ActionComponent,
	const FMVAICombatResolvedAction& Candidate,
	FName& OutActionTableName,
	FName& OutActionRowName)
{
	OutActionTableName = ExecuteAttackActionTableNameFromDataTable(Candidate.ActionRow.DataTable);
	OutActionRowName = Candidate.ActionRow.RowName;
	if (!MVAICombat::HasExecutableActionRow(Candidate))
	{
		return false;
	}

	return ActionComponent.TryStartActionFromRowHandle(Candidate.ActionRow, Candidate.StartSection);
}

bool ExecuteAttackIsStartedActionRunning(
	const UMVActionComponent& ActionComponent,
	const FName StartedActionTableName,
	const FName StartedActionRowName)
{
	return ActionComponent.IsActionRunning()
		&& ActionComponent.GetActiveActionTableName() == StartedActionTableName
		&& ActionComponent.GetActiveActionRowName() == StartedActionRowName;
}

bool ExecuteAttackStartCooldown(APawn& Owner, const FMVAICombatResolvedAction& Candidate)
{
	UMVActionCooldownComponent* CooldownComponent = Owner.FindComponentByClass<UMVActionCooldownComponent>();
	if (!CooldownComponent)
	{
		return true;
	}

	const FName CooldownActionId = MVAICombat::MakeCooldownActionId(Candidate);
	return CooldownActionId.IsNone() || CooldownComponent->StartCooldown(CooldownActionId);
}

bool ExecuteAttackCanSelectCandidate(
	const FMVAICombatContext& CombatContext,
	const FMVAICombatActionCondition& Candidate,
	const bool bAllowRepeatedLastAttack)
{
	const FMVActionRequest ActionRequest = ExecuteAttackResolveActionRequest(Candidate.ActionRequest);
	if (ActionRequest.Domain != EMVActionDomain::Attack || ActionRequest.RowName.IsNone())
	{
		return false;
	}

	if (!MVAICombat::IsDistanceInRange(CombatContext, Candidate.MinDistance, Candidate.MaxDistance))
	{
		return false;
	}

	if (FMath::Abs(CombatContext.AngleToTarget) > Candidate.MaxAbsAngle)
	{
		return false;
	}

	if (!MVAICombat::IsActionReady(
		CombatContext,
		MVAICombat::MakeCooldownActionId(Candidate.Metadata, ActionRequest)))
	{
		return false;
	}

	if (!bAllowRepeatedLastAttack && ActionRequest.RowName == CombatContext.LastAttackTag)
	{
		return false;
	}

	if (Candidate.bRequiresLineOfSight && !CombatContext.bHasLineOfSight)
	{
		return false;
	}

	if (Candidate.bRequiresForwardPathClear && !CombatContext.bSprintPathClear)
	{
		return false;
	}

	if (Candidate.bRequiresBackwardPathClear && !CombatContext.bStrafePathClear)
	{
		return false;
	}

	if (Candidate.bRequiresTargetCanBeAirborne && !CombatContext.bTargetCanBeAirborne)
	{
		return false;
	}

	if (Candidate.bRequiresTargetNotAirborne && CombatContext.bTargetIsAirborne)
	{
		return false;
	}

	if (Candidate.bRequiresAirbornePattern && !CombatContext.bShouldUseAirborneCharge)
	{
		return false;
	}

	return true;
}

bool ExecuteAttackSelectCandidate(
	const FMVAICombatContext& CombatContext,
	const TArray<FMVAICombatActionCondition>& Candidates,
	FMVAICombatActionCondition& OutCandidate)
{
	for (const FMVAICombatActionCondition& Candidate : Candidates)
	{
		if (ExecuteAttackCanSelectCandidate(CombatContext, Candidate, false))
		{
			OutCandidate = Candidate;
			return true;
		}
	}

	for (const FMVAICombatActionCondition& Candidate : Candidates)
	{
		if (ExecuteAttackCanSelectCandidate(CombatContext, Candidate, true))
		{
			OutCandidate = Candidate;
			return true;
		}
	}

	return false;
}
}

FMVExecuteFixedAttackTask::FMVExecuteFixedAttackTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVExecuteFixedAttackTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Enemy && InstanceData.AttackMontageEndedHandle.IsValid())
	{
		InstanceData.Enemy->OnAttackMontageEnded.Remove(InstanceData.AttackMontageEndedHandle);
	}

	InstanceData.ActionComponent = nullptr;
	InstanceData.Enemy = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;
	InstanceData.AttackInstanceId = INDEX_NONE;
	InstanceData.AttackMontageEndedHandle.Reset();
	InstanceData.ChooserAttackActionRowHandle.Reset();

	APawn* Owner = ExecuteAttackResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	FMVAICombatResolvedAction ResolvedAttack;
	FMVActionRequest ResolvedActionRequest = ExecuteAttackResolveActionRequest(InstanceData.ActionRequest);
	if (!ExecuteAttackResolveActionCandidate(
		*Owner,
		InstanceData.AttackMetadata,
		ResolvedActionRequest,
		InstanceData.AttackChooserTable,
		InstanceData.ChooserAttackActionRowHandle,
		ResolvedAttack))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (InstanceData.ActionComponent)
	{
		if (!ExecuteAttackTryStartAction(
			*InstanceData.ActionComponent,
			ResolvedAttack,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
		{
			return EStateTreeRunStatus::Failed;
		}

		if (!ExecuteAttackStartCooldown(*Owner, ResolvedAttack))
		{
			InstanceData.ActionComponent->CancelActiveAction(0.0f);
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.LastAttackTag = MVAICombat::MakeActionTag(ResolvedAttack);
		return EStateTreeRunStatus::Running;
	}

	InstanceData.Enemy = Cast<AMVEnemy>(Owner);
	if (!InstanceData.Enemy || !InstanceData.Enemy->Attack(InstanceData.FallbackAttackDirection, InstanceData.AttackInstanceId))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!ExecuteAttackStartCooldown(*Owner, ResolvedAttack))
	{
		return EStateTreeRunStatus::Failed;
	}

	const int32 ExpectedAttackInstanceId = InstanceData.AttackInstanceId;
	InstanceData.AttackMontageEndedHandle = InstanceData.Enemy->OnAttackMontageEnded.AddLambda(
		[WeakContext = Context.MakeWeakExecutionContext(), ExpectedAttackInstanceId](
			const int32 FinishedAttackInstanceId,
			UAnimMontage* Montage,
			const bool bInterrupted)
		{
			if (FinishedAttackInstanceId == ExpectedAttackInstanceId)
			{
				WeakContext.FinishTask(bInterrupted
					? EStateTreeFinishTaskType::Failed
					: EStateTreeFinishTaskType::Succeeded);
			}
		});

	InstanceData.LastAttackTag = MVAICombat::MakeActionTag(ResolvedAttack);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVExecuteFixedAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Running;
	}

	return ExecuteAttackIsStartedActionRunning(
		*InstanceData.ActionComponent,
		InstanceData.StartedActionTableName,
		InstanceData.StartedActionRowName)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVExecuteFixedAttackTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.Enemy && InstanceData.AttackMontageEndedHandle.IsValid())
	{
		InstanceData.Enemy->OnAttackMontageEnded.Remove(InstanceData.AttackMontageEndedHandle);
		InstanceData.AttackMontageEndedHandle.Reset();
	}

	if (InstanceData.ActionComponent
		&& ExecuteAttackIsStartedActionRunning(
			*InstanceData.ActionComponent,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
	{
		InstanceData.ActionComponent->CancelActiveAction(0.1f);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}

FMVSelectAndExecuteAttackTask::FMVSelectAndExecuteAttackTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FMVSelectAndExecuteAttackTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	InstanceData.ActionComponent = nullptr;
	InstanceData.StartedActionTableName = NAME_None;
	InstanceData.StartedActionRowName = NAME_None;
	InstanceData.SelectedActionRequest.Reset();
	InstanceData.SelectedMetadata = FMVAICombatActionMetadata();
	InstanceData.ChooserAttackActionRowHandle.Reset();

	APawn* Owner = ExecuteAttackResolveOwner(Context, InstanceData.Owner);
	if (!Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	FMVAICombatActionCondition SelectedCandidate;
	if (!ExecuteAttackSelectCandidate(InstanceData.CombatContext, InstanceData.Candidates, SelectedCandidate))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.SelectedActionRequest = ExecuteAttackResolveActionRequest(SelectedCandidate.ActionRequest);
	InstanceData.SelectedMetadata = SelectedCandidate.Metadata;

	FMVAICombatResolvedAction ResolvedAttack;
	if (!ExecuteAttackResolveActionCandidate(
		*Owner,
		InstanceData.SelectedMetadata,
		InstanceData.SelectedActionRequest,
		InstanceData.AttackChooserTable,
		InstanceData.ChooserAttackActionRowHandle,
		ResolvedAttack))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ActionComponent = Owner->FindComponentByClass<UMVActionComponent>();
	if (!InstanceData.ActionComponent
		|| !ExecuteAttackTryStartAction(
			*InstanceData.ActionComponent,
			ResolvedAttack,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!ExecuteAttackStartCooldown(*Owner, ResolvedAttack))
	{
		InstanceData.ActionComponent->CancelActiveAction(0.0f);
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.LastAttackTag = MVAICombat::MakeActionTag(ResolvedAttack);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMVSelectAndExecuteAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!InstanceData.ActionComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	return ExecuteAttackIsStartedActionRunning(
		*InstanceData.ActionComponent,
		InstanceData.StartedActionTableName,
		InstanceData.StartedActionRowName)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FMVSelectAndExecuteAttackTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.ActionComponent
		&& ExecuteAttackIsStartedActionRunning(
			*InstanceData.ActionComponent,
			InstanceData.StartedActionTableName,
			InstanceData.StartedActionRowName))
	{
		InstanceData.ActionComponent->CancelActiveAction(0.1f);
	}

	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}
