// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVStatusEffectComponent.h"

#include "Engine/World.h"
#include "StatusEffects/MVStatusEffectBehavior.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "UObject/UObjectGlobals.h"

UMVStatusEffectComponent::UMVStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

FMVStatusEffectHandle UMVStatusEffectComponent::FindStatusEffectHandle(const UMVStatusEffectDefinition* Definition,
	const AActor* SourceActor) const
{
	if (!::IsValid(Definition))
	{
		return FMVStatusEffectHandle();
	}

	const UWorld* World = GetWorld();

	if (!World)
	{
		return FMVStatusEffectHandle();
	}

	const double CurrentTime = World->GetTimeSeconds();

	for (const FMVStatusEffectInstance& Instance : ActiveEffects)
	{
		if (!Instance.IsValid() || Instance.IsExpired(CurrentTime))
		{
			continue;
		}

		if (Instance.Definition.Get() != Definition)
		{
			continue;
		}

		if (SourceActor && Instance.SourceActor.Get() != SourceActor)
		{
			continue;
		}

		return Instance.Handle;
	}

	return FMVStatusEffectHandle();
}

bool UMVStatusEffectComponent::HasStatusEffect(const UMVStatusEffectDefinition* Definition,
	const AActor* SourceActor) const
{
	return FindStatusEffectHandle(Definition, SourceActor).IsValid();
}

bool UMVStatusEffectComponent::SetStatusEffectStacks(FMVStatusEffectHandle Handle, int32 NewStacks)
{
	if (!Handle.IsValid() || NewStacks <= 0)
	{
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	FMVStatusEffectInstance* Instance = ActiveEffects.FindByPredicate(
		[Handle](const FMVStatusEffectInstance& ActiveInstance)
		{
			return ActiveInstance.Handle == Handle;
		});

	if (!Instance || !Instance->IsValid())
	{
		return false;
	}

	if (Instance->IsExpired(World->GetTimeSeconds()))
	{
		return false;
	}

	const UMVStatusEffectDefinition* Definition = Instance->Definition.Get();

	if (!::IsValid(Definition))
	{
		return false;
	}

	if (Definition->StackPolicy != EMVStatusEffectStackPolicy::AddStack)
	{
		return false;
	}

	const int32 SafeMaxStacks = FMath::Max(1, Definition->MaxStacks);
	const int32 ClampedStacks = FMath::Clamp(NewStacks, 1, SafeMaxStacks);
	const int32 PreviousStacks = Instance->CurrentStacks;

	if (PreviousStacks == ClampedStacks)
	{
		return true;
	}

	Instance->CurrentStacks = ClampedStacks;

	const FMVStatusEffectInstance UpdatedSnapshot = *Instance;
	OnStatusEffectUpdated.Broadcast(UpdatedSnapshot, PreviousStacks);

	return true;
}

FMVStatusEffectHandle UMVStatusEffectComponent::ApplyStatusEffect(const FMVStatusEffectSpec& Spec)
{
	if (!Spec.IsValid())
	{
		return FMVStatusEffectHandle();
	}

	const UMVStatusEffectDefinition* Definition = Spec.Definition.Get();
	if (!Definition)
	{
		return FMVStatusEffectHandle();
	}

	if (Definition->DurationPolicy == EMVStatusEffectDurationPolicy::Instant)
	{
		return FMVStatusEffectHandle();
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return FMVStatusEffectHandle();
	}

	const double CurrentTime = World->GetTimeSeconds();
	RemoveExpiredEffects(CurrentTime);

	FMVStatusEffectHandle ResultHandle;
	if (FMVStatusEffectInstance* ExistingInstance = FindMatchingInstance(Spec))
	{
		const int32 PreviousStacks = ExistingInstance->CurrentStacks;

		ReapplyToInstance(*ExistingInstance, Spec, CurrentTime);

		const FMVStatusEffectInstance UpdatedSnapshot = *ExistingInstance;
		ResultHandle = UpdatedSnapshot.Handle;

		OnStatusEffectUpdated.Broadcast(UpdatedSnapshot, PreviousStacks);

		NotifyBehaviorsUpdated(UpdatedSnapshot, PreviousStacks);
	}
	else
	{
		ResultHandle = CreateInstance(Spec, CurrentTime);
	}

	ScheduleNextExpiration();

	return ResultHandle;
}

bool UMVStatusEffectComponent::RemoveStatusEffect(FMVStatusEffectHandle Handle, EMVStatusEffectRemovalReason RemovalReason)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const int32 InstanceIndex = ActiveEffects.IndexOfByPredicate(
		[Handle](const FMVStatusEffectInstance& Instance)
		{
			return Instance.Handle == Handle;
		});

	if (InstanceIndex == INDEX_NONE)
	{
		return false;
	}

	if (!RemoveInstanceAtIndex(InstanceIndex, RemovalReason))
	{
		return false;
	}

	ScheduleNextExpiration();
	return true;
}

int32 UMVStatusEffectComponent::RemoveAllStatusEffects(EMVStatusEffectRemovalReason RemovalReason)
{
	const int32 RemovedCount = ActiveEffects.Num();

	for (int32 Index = ActiveEffects.Num() - 1; Index >= 0; Index--)
	{
		RemoveInstanceAtIndex(Index, RemovalReason);
	}

	ScheduleNextExpiration();

	return RemovedCount;
}

void UMVStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAllStatusEffects(EMVStatusEffectRemovalReason::OwnerEnded);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

FMVStatusEffectHandle UMVStatusEffectComponent::GenerateHandle()
{
	FMVStatusEffectHandle NewHandle;
	NewHandle.Value = NextHandleValue++;

	return NewHandle;
}

FMVStatusEffectInstance* UMVStatusEffectComponent::FindMatchingInstance(const FMVStatusEffectSpec& Spec)
{
	const UMVStatusEffectDefinition* Definition = Spec.Definition.Get();
	if (!Definition)
	{
		return nullptr;
	}

	if (Definition->InstanceScope == EMVStatusEffectInstanceScope::Independent)
	{
		return nullptr;
	}

	for (FMVStatusEffectInstance& Instance : ActiveEffects)
	{
		if (Instance.Definition.Get() != Definition)
		{
			continue;
		}

		switch (Definition->InstanceScope)
		{
		case EMVStatusEffectInstanceScope::OnePerTarget:
			return &Instance;

		case EMVStatusEffectInstanceScope::OnePerSource:
			if (Instance.SourceActor.Get() == Spec.SourceActor.Get())
			{
				return &Instance;
			}
			break;

		case EMVStatusEffectInstanceScope::Independent:
			break;
		}
	}

	return nullptr;
}

FMVStatusEffectHandle UMVStatusEffectComponent::CreateInstance(const FMVStatusEffectSpec& Spec, double CurrentTime)
{
	const UMVStatusEffectDefinition* Definition = Spec.Definition.Get();
	if (!Definition)
	{
		return FMVStatusEffectHandle();
	}

	FMVStatusEffectInstance NewInstance;

	NewInstance.Handle = GenerateHandle();
	NewInstance.Definition = Spec.Definition;
	NewInstance.SourceActor = Spec.SourceActor;
	NewInstance.AppliedTime = CurrentTime;
	NewInstance.ExpireTime = CalculateInitialExpireTime(*Definition, CurrentTime);

	if (Definition->StackPolicy == EMVStatusEffectStackPolicy::AddStack)
	{
		const int32 SafeMaxStacks = FMath::Max(1, Definition->MaxStacks);
		NewInstance.CurrentStacks = FMath::Clamp(Spec.StackDelta, 1, SafeMaxStacks);
	}
	else
	{
		NewInstance.CurrentStacks = 1;
	}

	CreateRuntimeBehaviors(NewInstance);

	const int32 AddedIndex = ActiveEffects.Add(MoveTemp(NewInstance));
	const FMVStatusEffectInstance AddedSnapShot = ActiveEffects[AddedIndex];

	OnStatusEffectAdded.Broadcast(AddedSnapShot);
	NotifyBehaviorsApplied(AddedSnapShot);

	return AddedSnapShot.Handle;
}

void UMVStatusEffectComponent::ReapplyToInstance(FMVStatusEffectInstance& Instance, const FMVStatusEffectSpec& Spec, double CurrentTime)
{
	const UMVStatusEffectDefinition* Definition = Spec.Definition.Get();
	if (!Definition)
	{
		return;
	}

	switch (Definition->StackPolicy)
	{
	case EMVStatusEffectStackPolicy::NoStack:
		break;

	case EMVStatusEffectStackPolicy::AddStack:
		{
			const int32 SafeMaxStacks = FMath::Max(1, Definition->MaxStacks);
			Instance.CurrentStacks = FMath::Clamp(Instance.CurrentStacks + Spec.StackDelta, 1, SafeMaxStacks);
		}
		break;

	case EMVStatusEffectStackPolicy::Replace:
		Instance.SourceActor = Spec.SourceActor;
		Instance.CurrentStacks = 1;
		Instance.AppliedTime = CurrentTime;
		Instance.ExpireTime = CalculateInitialExpireTime(*Definition, CurrentTime);
		return;
	}

	if (Definition->DurationPolicy == EMVStatusEffectDurationPolicy::Infinite)
	{
		Instance.ExpireTime = -1.0;
		return;
	}

	if (Definition->DurationPolicy != EMVStatusEffectDurationPolicy::Timed)
	{
		return;
	}

	const double SafeDuration = FMath::Max(0.0, static_cast<double>(Definition->Duration));

	switch (Definition->RefreshPolicy)
	{
	case EMVStatusEffectRefreshPolicy::NoRefresh:
		break;

	case EMVStatusEffectRefreshPolicy::RefreshDuration:
		Instance.ExpireTime = CurrentTime + SafeDuration;
		break;

	case EMVStatusEffectRefreshPolicy::ExtendDuration:
		Instance.ExpireTime = FMath::Max(Instance.ExpireTime, CurrentTime) + SafeDuration;
		break;
	}
}

void UMVStatusEffectComponent::RemoveExpiredEffects(double CurrentTime)
{
	for (int32 Index = ActiveEffects.Num() - 1; Index >= 0; Index--)
	{
		const FMVStatusEffectInstance& Instance = ActiveEffects[Index];

		if (!Instance.IsValid())
		{
			RemoveInstanceAtIndex(Index, EMVStatusEffectRemovalReason::Invalidated);
			continue;
		}

		if (Instance.IsExpired(CurrentTime))
		{
			RemoveInstanceAtIndex(Index, EMVStatusEffectRemovalReason::Expired);
		}
	}
}

double UMVStatusEffectComponent::CalculateInitialExpireTime(const UMVStatusEffectDefinition& Definition, double CurrentTime) const
{
	if (Definition.DurationPolicy == EMVStatusEffectDurationPolicy::Timed)
	{
		const double SafeDuration = FMath::Max(0.0, static_cast<double>(Definition.Duration));
		return CurrentTime + SafeDuration;
	}

	return -1.0;
}

void UMVStatusEffectComponent::ScheduleNextExpiration()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(ExpirationTimerHandle);

	double EarliestExpireTime = -1.0;

	for (const FMVStatusEffectInstance& Instance : ActiveEffects)
	{
		if (!Instance.IsValid() || !Instance.HasExpiration())
		{
			continue;
		}

		if (EarliestExpireTime < 0.0 || Instance.ExpireTime < EarliestExpireTime)
		{
			EarliestExpireTime = Instance.ExpireTime;
		}
	}

	if (EarliestExpireTime < 0.0)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	const double RemainingTime = FMath::Max(EarliestExpireTime - CurrentTime, 0.001);

	TimerManager.SetTimer(
		ExpirationTimerHandle,
		this,
		&UMVStatusEffectComponent::HandleExpirationTimer,
		static_cast<float>(RemainingTime),
		false
	);
}

void UMVStatusEffectComponent::HandleExpirationTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	RemoveExpiredEffects(CurrentTime);

	ScheduleNextExpiration();
}

bool UMVStatusEffectComponent::RemoveInstanceAtIndex(int32 Index, EMVStatusEffectRemovalReason RemovalReason)
{
	if (!ActiveEffects.IsValidIndex(Index))
	{
		return false;
	}

	const FMVStatusEffectInstance RemovedInstance = ActiveEffects[Index];

	ActiveEffects.RemoveAt(Index);

	OnStatusEffectRemoved.Broadcast(RemovedInstance, RemovalReason);
	NotifyBehaviorsRemoved(RemovedInstance, RemovalReason);

	return true;
}

void UMVStatusEffectComponent::CreateRuntimeBehaviors(FMVStatusEffectInstance& Instance)
{
	const UMVStatusEffectDefinition* Definition = Instance.Definition.Get();

	if (!Definition)
	{
		return;
	}

	for (UMVStatusEffectBehavior* BehaviorTemplate : Definition->BehaviorTemplates)
	{
		if (!::IsValid(BehaviorTemplate))
		{
			continue;
		}

		UMVStatusEffectBehavior* RuntimeBehavior = DuplicateObject<UMVStatusEffectBehavior>(BehaviorTemplate, this);

		if (!::IsValid(RuntimeBehavior))
		{
			continue;
		}

		RuntimeBehavior->InitializeBehavior(this, Instance.Handle);
		Instance.RuntimeBehaviors.Add(RuntimeBehavior);
	}
}

void UMVStatusEffectComponent::NotifyBehaviorsApplied(const FMVStatusEffectInstance& Instance)
{
	for (UMVStatusEffectBehavior* RuntimeBehavior : Instance.RuntimeBehaviors)
	{
		if (!::IsValid(RuntimeBehavior))
		{
			continue;
		}

		if (!RuntimeBehavior->GetEffectHandle().IsValid())
		{
			break;
		}

		RuntimeBehavior->OnApplied(Instance);
	}
}

void UMVStatusEffectComponent::NotifyBehaviorsUpdated(const FMVStatusEffectInstance& Instance, int32 PreviousStacks)
{
	for (UMVStatusEffectBehavior* RuntimeBehavior : Instance.RuntimeBehaviors)
	{
		if (!::IsValid(RuntimeBehavior))
		{
			continue;
		}

		if (!RuntimeBehavior->GetEffectHandle().IsValid())
		{
			break;
		}

		RuntimeBehavior->OnUpdated(Instance, PreviousStacks);
	}
}

void UMVStatusEffectComponent::NotifyBehaviorsRemoved(const FMVStatusEffectInstance& Instance,
	EMVStatusEffectRemovalReason RemovalReason)
{
	for (UMVStatusEffectBehavior* RuntimeBehavior : Instance.RuntimeBehaviors)
	{
		if (!::IsValid(RuntimeBehavior))
		{
			continue;
		}

		RuntimeBehavior->OnRemoved(Instance, RemovalReason);
		RuntimeBehavior->DeinitializeBehavior();
	}
}
