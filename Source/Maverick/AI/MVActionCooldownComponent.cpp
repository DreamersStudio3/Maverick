#include "AI/MVActionCooldownComponent.h"

UMVActionCooldownComponent::UMVActionCooldownComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVActionCooldownComponent::ConfigureCooldowns(const TArray<FMVActionCooldownDefinition>& Definitions)
{
	for (const FMVActionCooldownDefinition& Definition : Definitions)
	{
		if (Definition.ActionId.IsNone())
		{
			continue;
		}

		FMVActionCooldownState& State = CooldownStates.FindOrAdd(Definition.ActionId);
		const float NewDuration = FMath::Max(0.0f, Definition.CooldownDuration);

		if (State.CooldownDuration <= 0.0f && State.RemainingTime <= 0.0f)
		{
			State.RemainingTime = Definition.bStartReady ? 0.0f : NewDuration;
			State.bIsReady = Definition.bStartReady || NewDuration <= 0.0f;
		}
		else
		{
			State.RemainingTime = FMath::Min(State.RemainingTime, NewDuration);
			State.bIsReady = State.RemainingTime <= 0.0f;
		}

		State.CooldownDuration = NewDuration;
	}
}

void UMVActionCooldownComponent::TickCooldowns(const float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	for (TPair<FName, FMVActionCooldownState>& Cooldown : CooldownStates)
	{
		FMVActionCooldownState& State = Cooldown.Value;
		if (State.bIsReady)
		{
			continue;
		}

		State.RemainingTime = FMath::Max(0.0f, State.RemainingTime - DeltaTime);
		State.bIsReady = State.RemainingTime <= 0.0f;
	}
}

bool UMVActionCooldownComponent::StartCooldown(const FName ActionId)
{
	if (ActionId.IsNone())
	{
		return false;
	}

	FMVActionCooldownState* State = CooldownStates.Find(ActionId);
	if (!State)
	{
		return false;
	}

	State->RemainingTime = State->CooldownDuration;
	State->bIsReady = State->CooldownDuration <= 0.0f;
	return true;
}

bool UMVActionCooldownComponent::IsCooldownReady(const FName ActionId) const
{
	const FMVActionCooldownState* State = CooldownStates.Find(ActionId);
	return State ? State->bIsReady : false;
}

float UMVActionCooldownComponent::GetRemainingTime(const FName ActionId) const
{
	const FMVActionCooldownState* State = CooldownStates.Find(ActionId);
	return State ? State->RemainingTime : 0.0f;
}

void UMVActionCooldownComponent::GetReadyActionIds(TArray<FName>& OutActionIds) const
{
	OutActionIds.Reset();
	for (const TPair<FName, FMVActionCooldownState>& Cooldown : CooldownStates)
	{
		if (Cooldown.Value.bIsReady)
		{
			OutActionIds.Add(Cooldown.Key);
		}
	}
}
