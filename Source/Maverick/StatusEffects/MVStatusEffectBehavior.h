#pragma once

#include "CoreMinimal.h"
#include "Enum/MVStatusEffectEnums.h"
#include "Struct/MVStatusEffectTypes.h"
#include "UObject/Object.h"
#include "MVStatusEffectBehavior.generated.h"

class UWorld;
class UMVStatusEffectComponent;

/**
 * 상태 효과가 실행할 실제 행동의 기반 객체
 *
 * Definition에 편집 가능한 템플릿으로 보관
 * 런타임 상태 효과 생성 시 템플릿을 복제해 인스턴스별 행동 객체 생성
 *
 * 수명주기:
 * 1. 상태 효과 인스턴스 생성 시 InitializeBehavior
 * 2. 최초 적용 시 OnApplied
 * 3. 스택 또는 지속시간 갱신 시 OnUpdated
 * 4. 만료·소비·해제 시 OnRemoved
 * 5. 제거 처리 종료 시 DeinitializeBehavior
 */

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class MAVERICK_API UMVStatusEffectBehavior : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	void InitializeBehavior(UMVStatusEffectComponent* InOwningComponent, FMVStatusEffectHandle InEffectHandle);

	void DeinitializeBehavior();

	UFUNCTION(BlueprintNativeEvent, Category = "Maverick|StatusEffect|Behavior")
	void OnApplied(const FMVStatusEffectInstance& Instance);

	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance);

	UFUNCTION(BlueprintNativeEvent, Category = "Maverick|StatusEffect|Behavior")
	void OnUpdated(const FMVStatusEffectInstance& Instance, int32 PreviousStacks);

	virtual void OnUpdated_Implementation(const FMVStatusEffectInstance& Instance, int32 PreviousStacks);

	UFUNCTION(BlueprintNativeEvent, Category = "Maverick|StatusEffect|Behavior")
	void OnRemoved(const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason);

	virtual void OnRemoved_Implementation(const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason);

	UFUNCTION(BlueprintPure, Category = "Maverick|StatusEffect|Behavior")
	UMVStatusEffectComponent* GetOwningStatusEffectComponent() const { return OwningComponent.Get(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|StatusEffect|Behavior")
	FMVStatusEffectHandle GetEffectHandle() const { return EffectHandle; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UMVStatusEffectComponent> OwningComponent;

	UPROPERTY(Transient)
	FMVStatusEffectHandle EffectHandle;
};
