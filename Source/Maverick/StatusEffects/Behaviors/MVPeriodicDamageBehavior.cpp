// Fill out your copyright notice in the Description page of Project Settings.

#include "StatusEffects/Behaviors/MVPeriodicDamageBehavior.h"

#include "Components/MVStatusEffectComponent.h"
#include "Engine/World.h"
#include "StatusEffects/MVStatusEffectDamageUtility.h"
#include "Struct/MVHitTypes.h"

void UMVPeriodicDamageBehavior::OnApplied_Implementation(const FMVStatusEffectInstance& Instance)
{
	Super::OnApplied_Implementation(Instance);

	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	UMVStatusEffectComponent* StatusEffectComponent = GetOwningStatusEffectComponent();

	UWorld* World = GetWorld();

	if (!IsValid(StatusEffectComponent) || !IsValid(World))
	{
		return;
	}

	CachedSourceActor = Instance.SourceActor.Get();
	CachedTargetActor = StatusEffectComponent->GetOwner();
	CachedExpireTime = Instance.ExpireTime;
	NextTickTime = World->GetTimeSeconds() + GetSafeTickInterval();

	ScheduleNextDamageTick();
}

void UMVPeriodicDamageBehavior::OnUpdated_Implementation(const FMVStatusEffectInstance& Instance, const int32 PreviousStacks)
{
	Super::OnUpdated_Implementation(Instance, PreviousStacks);

	if (Instance.Handle != GetEffectHandle())
	{
		return;
	}

	UMVStatusEffectComponent* StatusEffectComponent = GetOwningStatusEffectComponent();

	if (!IsValid(StatusEffectComponent))
	{
		return;
	}

	CachedSourceActor = Instance.SourceActor.Get();
	CachedTargetActor = StatusEffectComponent->GetOwner();
	CachedExpireTime = Instance.ExpireTime;

	ScheduleNextDamageTick();
}

void UMVPeriodicDamageBehavior::OnRemoved_Implementation(const FMVStatusEffectInstance& Instance, const EMVStatusEffectRemovalReason RemovalReason)
{
	Super::OnRemoved_Implementation(Instance, RemovalReason);

	if (RemovalReason == EMVStatusEffectRemovalReason::Expired &&
		Instance.Handle == GetEffectHandle() &&
		NextTickTime >= 0.0 &&
		Instance.HasExpiration() &&
		NextTickTime <= Instance.ExpireTime + KINDA_SMALL_NUMBER)
	{
		ApplyDamageTick();
	}

	ClearDamageTimer();
}

double UMVPeriodicDamageBehavior::GetSafeTickInterval() const
{
	return FMath::Max(0.001, static_cast<double>(TickInterval));
}

void UMVPeriodicDamageBehavior::ScheduleNextDamageTick()
{
	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(DamageTimerHandle);

	if (NextTickTime < 0.0)
	{
		return;
	}

	if (CachedExpireTime >= 0.0 && NextTickTime > CachedExpireTime + KINDA_SMALL_NUMBER)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	const double RemainingTime = FMath::Max(NextTickTime - CurrentTime, 0.001);

	TimerManager.SetTimer(
		DamageTimerHandle,
		this,
		&UMVPeriodicDamageBehavior::HandleDamageTick,
		static_cast<float>(RemainingTime),
		false
	);
}

void UMVPeriodicDamageBehavior::HandleDamageTick()
{
	ApplyDamageTick();

	NextTickTime += GetSafeTickInterval();

	ScheduleNextDamageTick();
}

void UMVPeriodicDamageBehavior::ApplyDamageTick()
{
	MVStatusEffectDamage::ApplyTargetMaxHealthDamage(
		this,
		CachedSourceActor.Get(),
		CachedTargetActor.Get(),
		TargetMaxHealthDamageRatioPerTick
	);
}

void UMVPeriodicDamageBehavior::ClearDamageTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	CachedSourceActor.Reset();
	CachedTargetActor.Reset();
	NextTickTime = -1.0;
	CachedExpireTime = -1.0;
}