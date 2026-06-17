#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "MVTargetEvaluator.generated.h"

//Evaluator가 사용할 데이터 구조체
USTRUCT()
struct FMVTargetEvaluatorData
{	
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Output" )
	TObjectPtr<APawn> Target;
	
};

USTRUCT(meta = (DisplayName = "Target Evaluator"), Category = "Targeting")
struct FMVTargetEvaluator : public FStateTreeEvaluatorBase
{
	
	GENERATED_BODY()
	
	//인스턴스 데이터 타입을 엔진에 알림
	using InstanceDataType = FMVTargetEvaluatorData;
	
	virtual const UScriptStruct* GetInstanceDataType() const override { return FMVTargetEvaluatorData::StaticStruct(); }	
	
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	
	
	
};
