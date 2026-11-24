#pragma once

#include "StateTreeConditionBase.h"
#include "StateTreeTaskBase.h"
#include "MVDuelistApproachTask.generated.h"

class UMVDuelistApproachComponent;

USTRUCT()
struct FMVDuelistApproachConditionData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category="Approach") bool bRequireLineOfSight = true;
};

/** 추적 중 전진 이동 가능 여부를 현재 대상·지면·경과 시간으로 검사한다. */
USTRUCT(meta=(DisplayName="Duelist Can Approach"))
struct FMVDuelistCanApproach : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FMVDuelistApproachConditionData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

USTRUCT()
struct FMVDuelistApproachTaskData
{
	GENERATED_BODY()
	TWeakObjectPtr<UMVDuelistApproachComponent> Approach;
	TWeakObjectPtr<AActor> Target;
};

/** 전진 대시 상태의 수명 관리. 종료 시 원래 대상의 실제 사거리로 공격·추적 전이를 선택한다. */
USTRUCT(meta=(DisplayName="Duelist Approach Dash"))
struct FMVDuelistApproachTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FMVDuelistApproachTaskData;
	FMVDuelistApproachTask();
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
