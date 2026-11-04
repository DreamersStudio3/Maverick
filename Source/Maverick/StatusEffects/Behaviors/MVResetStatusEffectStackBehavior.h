#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVResetStatusEffectStackBehavior.generated.h"

class UMVStatusEffectDefinition;

/**
 * 소유 상태 효과가 시간 만료로 제거될 때
 * 연결된 다른 상태 효과의 스택을 지정된 값으로 초기화하는 행동
 *
 * 연결 효과는 같은 StatusEffectComponent 안에서 검색
 * TargetDefinition과 SourceActor가 모두 일치하는 인스턴스만 변경
 *
 * 수명주기:
 * 1. 쿨타임 상태 효과 생성 시 런타임 Behavior 생성
 * 2. 쿨타임 만료 시 OnRemoved 호출
 * 3. 연결 효과 Handle 검색
 * 4. 연결 효과의 CurrentStacks 변경
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVResetStatusEffectStackBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnRemoved_Implementation(const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StackReset")
	TSoftObjectPtr<UMVStatusEffectDefinition> TargetDefinition = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StackReset", meta = (ClampMin = "1"))
	int32 ResetStacks = 1;
};