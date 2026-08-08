#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StateTreeTaskBase.h"
#include "MVFocusingTask.generated.h"

class AAIController;

USTRUCT()
struct FMVFocusingTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input|Target")
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Owner")
	TObjectPtr<APawn> Owner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|bCanFocus")
	bool bCanFocus = true;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> FocusController = nullptr;

	UPROPERTY(Transient)
	bool bFocusApplied = false;
};

/**
 * State 진입 시 AIController focus를 적용하고 유지하다가 Exit에서 해제하는 지속형 StateTree Task.
 * 성공 즉시 끝나는 Task가 아니므로 전투 부모나 이동·대기 State에 두고, 공격 중 강제 회전을 원하지 않으면
 * 공격 State에는 배치하지 않는다.
 */
USTRUCT(meta = (DisplayName = "Focusing Task"))
struct FMVFocusingTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMVFocusingTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
		) const override;

	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime
		) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
		) const override;
};
