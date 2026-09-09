// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Enum/MVStatusEffectEnums.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "MVStatusEffectDefinition.generated.h"

class UTexture2D;

/**
 * 상태 효과 한 종류의 변경되지 않는 설계 데이터
 *
 * 지속 방식, 중첩 규칙, 표시 정보를 보유
 * 런타임 스택, 남은 시간, 시전자, 적용 대상은 보유하지 않음
 * 이후 UMVStatusEffectComponent가 런타임 인스턴스를 생성할 때 참조
 */
UCLASS(BlueprintType)
class MAVERICK_API UMVStatusEffectDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Identity", meta = (Categories = "StatusEffect"))
	FGameplayTag EffectId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Identity", meta = (Categories = "StatusEffect"))
	FGameplayTagContainer EffectTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Duration")
	EMVStatusEffectDurationPolicy DurationPolicy = EMVStatusEffectDurationPolicy::Instant;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Duration",
		meta = (
			ClampMin = "0.0",
			Units = "s",
			EditCondition = "DurationPolicy == EMVStatusEffectDurationPolicy::Timed",
			EditConditionHides
		)
	)
	float Duration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Stack")
	EMVStatusEffectInstanceScope InstanceScope = EMVStatusEffectInstanceScope::OnePerSource;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Stack")
	EMVStatusEffectStackPolicy StackPolicy = EMVStatusEffectStackPolicy::NoStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Duration")
	EMVStatusEffectRefreshPolicy RefreshPolicy = EMVStatusEffectRefreshPolicy::NoRefresh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|Stack",
		meta = (
			ClampMin = "1",
			EditCondition = "StackPolicy == EMVStatusEffectStackPolicy::AddStack",
			EditConditionHides
		)
	)
	int32 MaxStacks = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|UI")
	bool bVisibleInUI = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|UI", meta = (EditCondition = "bVisibleInUI"))
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|StatusEffect|UI", meta = (EditCondition = "bVisibleInUI"))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "StatusEffectBehaviorTemplates")
	TArray<TObjectPtr<UMVStatusEffectBehavior>> BehaviorTemplates;
};
