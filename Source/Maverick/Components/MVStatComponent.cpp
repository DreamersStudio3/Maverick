#include "Components/MVStatComponent.h"

#include "Tables/MVTableManager.h"

namespace
{
	float MVStatNonNegative(float Value)
	{
		return FMath::Max(0.0f, Value);
	}

	float MVStatClampCurrent(float Value, float MaxValue)
	{
		return FMath::Clamp(Value, 0.0f, MVStatNonNegative(MaxValue));
	}
}

UMVStatComponent::UMVStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVStatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bLoadStatsOnBeginPlay && !StatTableName.IsNone() && !StatRowKey.IsEmpty())
	{
		LoadStatsFromTable();
	}
}

void UMVStatComponent::SetStatTableReference(FName InStatTableName, const FString& InStatRowKey)
{
	StatTableName = InStatTableName;
	StatRowKey = InStatRowKey;
}

bool UMVStatComponent::LoadStatsFromTable()
{
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager || StatTableName.IsNone() || StatRowKey.IsEmpty())
	{
		return false;
	}

	float LoadedMaxHP = 0.0f;
	float LoadedCurrentHP = 0.0f;
	float LoadedMaxStamina = 0.0f;
	float LoadedCurrentStamina = 0.0f;
	float LoadedStaminaRecoveryPerSecond = 0.0f;
	float LoadedStaminaRecoveryDelay = 0.0f;
	float LoadedMaxMP = 0.0f;
	float LoadedCurrentMP = 0.0f;
	float LoadedMPRecoveryPerSecond = 0.0f;
	float LoadedAttackSpeed = 0.0f;
	float LoadedWalkSpeed = 0.0f;
	float LoadedRunSpeed = 0.0f;
	float LoadedSprintSpeed = 0.0f;
	float LoadedDefence = 0.0f;
	float LoadedMaxGroggy = 0.0f;
	float LoadedCurrentGroggy = 0.0f;
	float LoadedGroggyRecoveryPerSecond = 0.0f;
	float LoadedGroggyRecoveryDelay = 0.0f;

	bool bLoaded = true;
	bLoaded &= TryReadFloat(TableManager, TEXT("MaxHP"), LoadedMaxHP);
	bLoaded &= TryReadFloat(TableManager, TEXT("CurrentHP"), LoadedCurrentHP);
	bLoaded &= TryReadFloat(TableManager, TEXT("MaxStamina"), LoadedMaxStamina);
	bLoaded &= TryReadFloat(TableManager, TEXT("CurrentStamina"), LoadedCurrentStamina);
	bLoaded &= TryReadFloat(TableManager, TEXT("StaminaRecoveryPerSecond"), LoadedStaminaRecoveryPerSecond);
	bLoaded &= TryReadFloat(TableManager, TEXT("StaminaRecoveryDelay"), LoadedStaminaRecoveryDelay);
	bLoaded &= TryReadFloat(TableManager, TEXT("MaxMP"), LoadedMaxMP);
	bLoaded &= TryReadFloat(TableManager, TEXT("CurrentMP"), LoadedCurrentMP);
	bLoaded &= TryReadFloat(TableManager, TEXT("MPRecoveryPerSecond"), LoadedMPRecoveryPerSecond);
	bLoaded &= TryReadFloat(TableManager, TEXT("AttackSpeed"), LoadedAttackSpeed);
	bLoaded &= TryReadFloat(TableManager, TEXT("WalkSpeed"), LoadedWalkSpeed);
	bLoaded &= TryReadFloat(TableManager, TEXT("RunSpeed"), LoadedRunSpeed);
	bLoaded &= TryReadFloat(TableManager, TEXT("SprintSpeed"), LoadedSprintSpeed);
	bLoaded &= TryReadFloat(TableManager, TEXT("Defence"), LoadedDefence);
	bLoaded &= TryReadFloat(TableManager, TEXT("MaxGroggy"), LoadedMaxGroggy);
	bLoaded &= TryReadFloat(TableManager, TEXT("CurrentGroggy"), LoadedCurrentGroggy);
	bLoaded &= TryReadFloat(TableManager, TEXT("GroggyRecoveryPerSecond"), LoadedGroggyRecoveryPerSecond);
	bLoaded &= TryReadFloat(TableManager, TEXT("GroggyRecoveryDelay"), LoadedGroggyRecoveryDelay);

	if (!bLoaded)
	{
		return false;
	}

	SetMaxHP(LoadedMaxHP);
	SetCurrentHP(LoadedCurrentHP);
	SetMaxStamina(LoadedMaxStamina);
	SetCurrentStamina(LoadedCurrentStamina);
	SetStaminaRecoveryPerSecond(LoadedStaminaRecoveryPerSecond);
	SetStaminaRecoveryDelay(LoadedStaminaRecoveryDelay);
	SetMaxMP(LoadedMaxMP);
	SetCurrentMP(LoadedCurrentMP);
	SetMPRecoveryPerSecond(LoadedMPRecoveryPerSecond);
	SetAttackSpeed(LoadedAttackSpeed);
	SetWalkSpeed(LoadedWalkSpeed);
	SetRunSpeed(LoadedRunSpeed);
	SetSprintSpeed(LoadedSprintSpeed);
	SetDefence(LoadedDefence);
	SetMaxGroggy(LoadedMaxGroggy);
	SetCurrentGroggy(LoadedCurrentGroggy);
	SetGroggyRecoveryPerSecond(LoadedGroggyRecoveryPerSecond);
	SetGroggyRecoveryDelay(LoadedGroggyRecoveryDelay);

	return true;
}

void UMVStatComponent::TickRecoverableStats(float DeltaTime)
{
	if (DeltaTime <= 0.0f || IsRecoverableStatRecoveryPaused())
	{
		return;
	}

	if (bUseRecoverableStatRecoveryDelay)
	{
		RecoverableStatCooldownRemaining = FMath::Max(0.0f, RecoverableStatCooldownRemaining - DeltaTime);
	}
	else
	{
		RecoverableStatCooldownRemaining = 0.0f;
	}

	if (RecoverableStatCooldownRemaining <= 0.0f)
	{
		RecoverStamina(StaminaRecoveryPerSecond * DeltaTime);
		RecoverMP(MPRecoveryPerSecond * DeltaTime);
	}
}

void UMVStatComponent::RestartRecoverableStatCooldown()
{
	RecoverableStatCooldownRemaining = bUseRecoverableStatRecoveryDelay
		? StaminaRecoveryDelay
		: 0.0f;
}

void UMVStatComponent::BeginRecoverableStatRecoveryPause()
{
	const bool bWasPaused = IsRecoverableStatRecoveryPaused();
	++RecoverableStatRecoveryPauseCount;
	if (!bWasPaused)
	{
		RecoverableStatCooldownRemaining = 0.0f;
		OnStatRecentLossHoldChanged.Broadcast(true);
	}
}

void UMVStatComponent::EndRecoverableStatRecoveryPause()
{
	if (RecoverableStatRecoveryPauseCount <= 0)
	{
		RecoverableStatRecoveryPauseCount = 0;
		return;
	}

	--RecoverableStatRecoveryPauseCount;
	if (RecoverableStatRecoveryPauseCount <= 0)
	{
		RecoverableStatRecoveryPauseCount = 0;
		RestartRecoverableStatCooldown();
		OnStatRecentLossHoldChanged.Broadcast(false);
	}
}

bool UMVStatComponent::IsRecoverableStatRecoveryPaused() const
{
	return RecoverableStatRecoveryPauseCount > 0;
}

void UMVStatComponent::SetMaxHP(float InMaxHP)
{
	const float PreviousMaxHP = MaxHP;
	const float PreviousCurrentHP = CurrentHP;
	MaxHP = MVStatNonNegative(InMaxHP);
	CurrentHP = MVStatClampCurrent(CurrentHP, MaxHP);

	if (!FMath::IsNearlyEqual(PreviousMaxHP, MaxHP) || !FMath::IsNearlyEqual(PreviousCurrentHP, CurrentHP))
	{
		OnHPChanged.Broadcast(CurrentHP, MaxHP);
	}

	if (PreviousCurrentHP > 0.0f && CurrentHP <= 0.0f)
	{
		OnDead.Broadcast();
	}
}

void UMVStatComponent::SetCurrentHP(float InCurrentHP)
{
	const float PreviousCurrentHP = CurrentHP;
	CurrentHP = MVStatClampCurrent(InCurrentHP, MaxHP);

	if (!FMath::IsNearlyEqual(PreviousCurrentHP, CurrentHP))
	{
		OnHPChanged.Broadcast(CurrentHP, MaxHP);
	}

	if (PreviousCurrentHP > 0.0f && CurrentHP <= 0.0f)
	{
		OnDead.Broadcast();
	}
}

void UMVStatComponent::SetMaxStamina(float InMaxStamina)
{
	const float PreviousMaxStamina = MaxStamina;
	const float PreviousCurrentStamina = CurrentStamina;
	MaxStamina = MVStatNonNegative(InMaxStamina);
	CurrentStamina = MVStatClampCurrent(CurrentStamina, MaxStamina);

	if (!FMath::IsNearlyEqual(PreviousMaxStamina, MaxStamina) || !FMath::IsNearlyEqual(PreviousCurrentStamina, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UMVStatComponent::SetCurrentStamina(float InCurrentStamina)
{
	const float PreviousCurrentStamina = CurrentStamina;
	CurrentStamina = MVStatClampCurrent(InCurrentStamina, MaxStamina);

	if (!FMath::IsNearlyEqual(PreviousCurrentStamina, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UMVStatComponent::SetStaminaRecoveryPerSecond(float InStaminaRecoveryPerSecond)
{
	StaminaRecoveryPerSecond = MVStatNonNegative(InStaminaRecoveryPerSecond);
}

void UMVStatComponent::SetStaminaRecoveryDelay(float InStaminaRecoveryDelay)
{
	StaminaRecoveryDelay = MVStatNonNegative(InStaminaRecoveryDelay);
}

bool UMVStatComponent::HasStamina(float RequiredAmount) const
{
	return CurrentStamina >= MVStatNonNegative(RequiredAmount);
}

bool UMVStatComponent::ConsumeStamina(float Amount)
{
	const float NormalizedAmount = MVStatNonNegative(Amount);
	if (NormalizedAmount <= 0.0f)
	{
		return true;
	}

	const bool bHadEnoughStamina = CurrentStamina >= NormalizedAmount;
	SetCurrentStamina(CurrentStamina - NormalizedAmount);
	RestartRecoverableStatCooldown();
	return bHadEnoughStamina;
}

void UMVStatComponent::RecoverStamina(float Amount)
{
	const float NormalizedAmount = MVStatNonNegative(Amount);
	if (NormalizedAmount <= 0.0f)
	{
		return;
	}

	SetCurrentStamina(CurrentStamina + NormalizedAmount);
}

void UMVStatComponent::SetMaxMP(float InMaxMP)
{
	const float PreviousMaxMP = MaxMP;
	const float PreviousCurrentMP = CurrentMP;
	MaxMP = MVStatNonNegative(InMaxMP);
	CurrentMP = MVStatClampCurrent(CurrentMP, MaxMP);

	if (!FMath::IsNearlyEqual(PreviousMaxMP, MaxMP) || !FMath::IsNearlyEqual(PreviousCurrentMP, CurrentMP))
	{
		OnMPChanged.Broadcast(CurrentMP, MaxMP);
	}
}

void UMVStatComponent::SetCurrentMP(float InCurrentMP)
{
	const float PreviousCurrentMP = CurrentMP;
	CurrentMP = MVStatClampCurrent(InCurrentMP, MaxMP);

	if (!FMath::IsNearlyEqual(PreviousCurrentMP, CurrentMP))
	{
		OnMPChanged.Broadcast(CurrentMP, MaxMP);
	}
}

void UMVStatComponent::SetMPRecoveryPerSecond(float InMPRecoveryPerSecond)
{
	MPRecoveryPerSecond = MVStatNonNegative(InMPRecoveryPerSecond);
}

bool UMVStatComponent::HasMP(float RequiredAmount) const
{
	return CurrentMP >= MVStatNonNegative(RequiredAmount);
}

bool UMVStatComponent::ConsumeMP(float Amount)
{
	const float NormalizedAmount = MVStatNonNegative(Amount);
	if (NormalizedAmount <= 0.0f)
	{
		return true;
	}

	const bool bHadEnoughMP = CurrentMP >= NormalizedAmount;
	SetCurrentMP(CurrentMP - NormalizedAmount);
	RestartRecoverableStatCooldown();
	return bHadEnoughMP;
}

void UMVStatComponent::RecoverMP(float Amount)
{
	const float NormalizedAmount = MVStatNonNegative(Amount);
	if (NormalizedAmount <= 0.0f)
	{
		return;
	}

	SetCurrentMP(CurrentMP + NormalizedAmount);
}

void UMVStatComponent::SetAttackSpeed(float InAttackSpeed)
{
	AttackSpeed = MVStatNonNegative(InAttackSpeed);
}

void UMVStatComponent::SetWalkSpeed(float InWalkSpeed)
{
	WalkSpeed = MVStatNonNegative(InWalkSpeed);
}

void UMVStatComponent::SetRunSpeed(float InRunSpeed)
{
	RunSpeed = MVStatNonNegative(InRunSpeed);
}

void UMVStatComponent::SetSprintSpeed(float InSprintSpeed)
{
	SprintSpeed = MVStatNonNegative(InSprintSpeed);
}

void UMVStatComponent::SetDefence(float InDefence)
{
	Defence = MVStatNonNegative(InDefence);
}

void UMVStatComponent::SetMaxGroggy(float InMaxGroggy)
{
	const float PreviousMaxGroggy = MaxGroggy;
	const float PreviousCurrentGroggy = CurrentGroggy;
	MaxGroggy = MVStatNonNegative(InMaxGroggy);
	CurrentGroggy = MVStatClampCurrent(CurrentGroggy, MaxGroggy);

	if (!FMath::IsNearlyEqual(PreviousMaxGroggy, MaxGroggy) || !FMath::IsNearlyEqual(PreviousCurrentGroggy, CurrentGroggy))
	{
		OnGroggyChanged.Broadcast(CurrentGroggy, MaxGroggy);
	}
}

void UMVStatComponent::SetCurrentGroggy(float InCurrentGroggy)
{
	const float PreviousCurrentGroggy = CurrentGroggy;
	CurrentGroggy = MVStatClampCurrent(InCurrentGroggy, MaxGroggy);

	if (!FMath::IsNearlyEqual(PreviousCurrentGroggy, CurrentGroggy))
	{
		OnGroggyChanged.Broadcast(CurrentGroggy, MaxGroggy);
	}
}

void UMVStatComponent::SetGroggyRecoveryPerSecond(float InGroggyRecoveryPerSecond)
{
	GroggyRecoveryPerSecond = MVStatNonNegative(InGroggyRecoveryPerSecond);
}

void UMVStatComponent::SetGroggyRecoveryDelay(float InGroggyRecoveryDelay)
{
	GroggyRecoveryDelay = MVStatNonNegative(InGroggyRecoveryDelay);
}

bool UMVStatComponent::TryReadFloat(const UMVTableManager* TableManager, const FString& FieldName, float& OutValue) const
{
	return TableManager && TableManager->GetFloat(StatTableName, StatRowKey, FieldName, OutValue);
}
