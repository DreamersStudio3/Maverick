#pragma once

#include "CoreMinimal.h"
#include "AI/MVAICombatTypes.h"
#include "AI/Enum/MVAttackDirection.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "Tables/MVActionTableTypes.h"
#include "UObject/SoftObjectPath.h"
#include "MVExecuteAttackTask.generated.h"

class AMVEnemy;
class UMVActionComponent;

USTRUCT()
struct FMVExecuteFixedAttackTaskInstanceData
{
	GENERATED_BODY()

	FMVExecuteFixedAttackTaskInstanceData()
	{
		ActionRequest.Domain = EMVActionDomain::Attack;
	}

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	FMVAICombatActionMetadata AttackMetadata;

	UPROPERTY(EditAnywhere, Category = "Input|Action")
	FMVActionRequest ActionRequest;

	UPROPERTY(EditAnywhere, Category = "Input|Attack|Chooser")
	FSoftObjectPath AttackChooserTable;

	UPROPERTY(EditAnywhere, Category = "Input|Attack")
	EMVAttackDirection FallbackAttackDirection = EMVAttackDirection::Forward;

	UPROPERTY(EditAnywhere, Category = "Output")
	FName LastAttackTag = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AMVEnemy> Enemy = nullptr;

	UPROPERTY(Transient)
	FMVAttackActionRowHandle ChooserAttackActionRowHandle;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
	int32 AttackInstanceId = INDEX_NONE;
	FDelegateHandle AttackMontageEndedHandle;
};

/**
 * 단일 `ActionRequest`를 Chooser와 DataTable Row로 해석해 ActionComponent에서 실행하는 StateTree Task.
 * 실행에 성공하면 후보 쿨다운을 시작하고 RowName 기반 `LastAttackTag`를 출력하며, 시작한 Action이 끝날 때까지
 * Running을 유지한다.
 */
USTRUCT(meta = (DisplayName = "Execute Fixed Attack Task"))
struct FMVExecuteFixedAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVExecuteFixedAttackTaskInstanceData;

	FMVExecuteFixedAttackTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct FMVSelectAndExecuteAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Context")
	FMVAICombatContext CombatContext;

	UPROPERTY(EditAnywhere, Category = "Input|Selection")
	FSoftObjectPath AttackChooserTable;

	UPROPERTY(EditAnywhere, Category = "Input|Selection")
	TArray<FMVAICombatActionCondition> Candidates;

	UPROPERTY(EditAnywhere, Category = "Output")
	FMVActionRequest SelectedActionRequest;

	UPROPERTY(EditAnywhere, Category = "Output")
	FMVAICombatActionMetadata SelectedMetadata;

	UPROPERTY(EditAnywhere, Category = "Output")
	FName LastAttackTag = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<UMVActionComponent> ActionComponent = nullptr;

	UPROPERTY(Transient)
	FMVAttackActionRowHandle ChooserAttackActionRowHandle;

	FName StartedActionTableName = NAME_None;
	FName StartedActionRowName = NAME_None;
};

/**
 * 여러 공격 후보 중 현재 snapshot에 맞는 첫 후보를 골라 실행하는 StateTree Task.
 *
 * 후보 배열 순서를 유지하되 첫 번째 탐색에서는 `LastAttackTag`와 같은 RowName을 피하고, 선택지가 없을 때만
 * 두 번째 탐색에서 반복을 허용한다. 빈 배열이나 실행 가능한 후보가 없으면 실패하며, 실행·쿨다운·완료
 * lifecycle은 Fixed Task와 같은 ActionComponent 경로를 사용한다.
 */
USTRUCT(meta = (DisplayName = "Select And Execute Attack Task"))
struct FMVSelectAndExecuteAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVSelectAndExecuteAttackTaskInstanceData;

	FMVSelectAndExecuteAttackTask();

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
