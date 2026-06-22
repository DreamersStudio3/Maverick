#include "Components/MVStatComponent.h"

#include "Tables/MVTableManager.h"
#include "Tables/MVStatTableTypes.h"

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

	if (bLoadStatsOnBeginPlay && !StatTableName.IsNone() && CharacterIndexId > 0)
	{
		LoadStatsFromTable();
	}
}

void UMVStatComponent::SetStatTableReference(FName InStatTableName, const FString& InStatRowKey)
{
	StatTableName = InStatTableName;

	int32 ParsedCharacterIndexId = 0;
	if (LexTryParseString(ParsedCharacterIndexId, *InStatRowKey))
	{
		SetCharacterIndexId(ParsedCharacterIndexId);
	}
}

void UMVStatComponent::SetCharacterIndexId(const int32 NewCharacterIndexId)
{
	CharacterIndexId = FMath::Max(0, NewCharacterIndexId);
}

int32 UMVStatComponent::GetCharacterIndexId() const
{
	return CharacterIndexId;
}

bool UMVStatComponent::LoadStatsFromTable()
{
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager || StatTableName.IsNone() || CharacterIndexId <= 0)
	{
		return false;
	}

	const FString StatRowKey = MakeStatRowKey();
	const FMVCharacterStatRow* StatRow = TableManager->FindRow<FMVCharacterStatRow>(StatTableName, StatRowKey);
	if (!StatRow)
	{
		return false;
	}

	if (StatRow->StatId != CharacterIndexId)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("CharacterStat row '%s' has StatId %d, expected CharacterIndexId %d."),
			*StatRowKey,
			StatRow->StatId,
			CharacterIndexId);
		return false;
	}

	SetMaxHP(StatRow->MaxHP);
	SetCurrentHP(StatRow->CurrentHP);
	SetMaxStamina(StatRow->MaxStamina);
	SetCurrentStamina(StatRow->CurrentStamina);
	SetStaminaRecoveryPerSecond(StatRow->StaminaRecoveryPerSecond);
	SetStaminaRecoveryDelay(StatRow->StaminaRecoveryDelay);
	SetMaxMP(StatRow->MaxMP);
	SetCurrentMP(StatRow->CurrentMP);
	SetMPRecoveryPerSecond(StatRow->MPRecoveryPerSecond);
	SetAttackSpeed(StatRow->AttackSpeed);
	SetWalkSpeed(StatRow->WalkSpeed);
	SetRunSpeed(StatRow->RunSpeed);
	SetSprintSpeed(StatRow->SprintSpeed);
	SetDefence(StatRow->Defence);
	SetMaxGroggy(StatRow->MaxGroggy);
	SetCurrentGroggy(StatRow->CurrentGroggy);
	SetGroggyRecoveryPerSecond(StatRow->GroggyRecoveryPerSecond);
	SetGroggyRecoveryDelay(StatRow->GroggyRecoveryDelay);

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

FString UMVStatComponent::MakeStatRowKey() const
{
	return FString::FromInt(CharacterIndexId);
}
