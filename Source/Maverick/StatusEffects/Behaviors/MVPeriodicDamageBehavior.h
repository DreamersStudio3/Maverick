#pragma once

#include "CoreMinimal.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "TimerManager.h"
#include "MVPeriodicDamageBehavior.generated.h"

class AActor;

/**
 * 상태 효과가 유지되는 동안 일정한 간격으로
 * 대상 최대 체력 비례 피해를 적용하는 행동
 *
 * 최초 피해는 적용 직후가 아닌 TickInterval 경과 후 발생
 * 상태 효과 갱신 시 피해 주기는 유지하고 종료 시각만 갱신
 * 상태 효과 제거 시 내부 피해 타이머 정리
 *
 * 만료 시각과 마지막 피해 시각이 같은 경우
 * 타이머 실행 순서와 관계없이 마지막 피해를 한 번 보장
 */
UCLASS(BlueprintType, EditInlineNew)
class MAVERICK_API UMVPeriodicDamageBehavior : public UMVStatusEffectBehavior
{
	GENERATED_BODY()

public:
	virtual void OnApplied_Implementation(const FMVStatusEffectInstance& Instance) override;

	virtual void OnUpdated_Implementation(const FMVStatusEffectInstance& Instance, int32 PreviousStacks) override;

	virtual void OnRemoved_Implementation(const FMVStatusEffectInstance& Instance, EMVStatusEffectRemovalReason RemovalReason) override;

private:
	double GetSafeTickInterval() const;

	void ScheduleNextDamageTick();

	void HandleDamageTick();

	void ApplyDamageTick();

	void ClearDamageTimer();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PeriodicDamage", meta = (ClampMin = "0.001", Units = "s"))
	float TickInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PeriodicDamage", meta = (ClampMin = "0.0"))
	float TargetMaxHealthDamageRatioPerTick = 0.001f;

private:
	FTimerHandle DamageTimerHandle;

	TWeakObjectPtr<AActor> CachedSourceActor;

	TWeakObjectPtr<AActor> CachedTargetActor;

	double NextTickTime = -1.0;

	double CachedExpireTime = -1.0;
};