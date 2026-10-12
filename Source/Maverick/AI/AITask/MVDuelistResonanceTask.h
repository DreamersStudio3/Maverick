#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "MVDuelistResonanceTask.generated.h"

class UMVDuelistResonanceComponent;

USTRUCT()
struct FMVDuelistResonanceTaskInstanceData
{
	GENERATED_BODY()
	UPROPERTY(Transient) TObjectPtr<UMVDuelistResonanceComponent> Resonance;
};

/** Resonanace 상태의 진입·실행 감시·중단 브리지, 그로기 종료까지 Running 유지; 수치·피해·연출은 Duelist 컴포넌트 책임 */
USTRUCT(meta=(DisplayName="Resonanace"))
struct MAVERICK_API FMVDuelistResonanceTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FMVDuelistResonanceTaskInstanceData;
	FMVDuelistResonanceTask();
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
