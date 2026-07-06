#include "Components/MVStatComponent.h"

#include "Tables/MVTableManager.h"
#include "Tables/MVStatTableTypes.h"
#include "Tags/MVGameplayTags.h"
#include "Components/MVActionComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMVStatComponent, Log, All);

namespace
{
	constexpr float MVStatMinimumMaxHP = 1.0f;

	float MVStatNonNegative(float Value)
	{
		return FMath::Max(0.0f, Value);
	}

	float MVStatClampMaxHP(float Value)
	{
		return FMath::Max(MVStatMinimumMaxHP, Value);
	}

	float MVStatClampCurrent(float Value, float MaxValue)
	{
		return FMath::Clamp(Value, 0.0f, MVStatNonNegative(MaxValue));
	}
}

UMVStatComponent::UMVStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CharacterIndexCode = MVGameplayTags::Character_Player_P1;
}

void UMVStatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bLoadStatsOnBeginPlay && !StatTableName.IsNone() && CharacterIndexCode.IsValid())
	{
		LoadStatsFromTable();
	}

	ActionCompRef = GetOwner()->FindComponentByClass<UMVActionComponent>();
	if(ActionCompRef)
	{
		ActionCompRef->OnStatPauseStart.Unbind();
		ActionCompRef->OnStatPauseStart.BindUObject(this, &UMVStatComponent::BeginRecoverableStatRecoveryPause);
		ActionCompRef->OnStatPauseEnd.Unbind();
		ActionCompRef->OnStatPauseEnd.BindUObject(this, &UMVStatComponent::EndRecoverableStatRecoveryPause);
	}
	else
	{
		UE_LOG(LogMVStatComponent, Warning, TEXT("No ActionComponent found on %s. Recoverable stat recovery pause will not work."), *GetOwner()->GetName());
	}

}

void UMVStatComponent::SetStatTableReference(FName InStatTableName, const FString& InStatRowKey)
{
	StatTableName = InStatTableName;

	SetCharacterIndexCode(FGameplayTag::RequestGameplayTag(FName(*InStatRowKey), false));
}

void UMVStatComponent::SetCharacterIndexCode(const FGameplayTag NewCharacterIndexCode)
{
	CharacterIndexCode = NewCharacterIndexCode;
}

FGameplayTag UMVStatComponent::GetCharacterIndexCode() const
{
	return CharacterIndexCode;
}

bool UMVStatComponent::LoadStatsFromTable()
{
	const UMVTableManager* TableManager = UMVTableManager::Get(this);
	if (!TableManager || StatTableName.IsNone() || !CharacterIndexCode.IsValid())
	{
		return false;
	}

	const FString StatRowKey = MakeStatRowKey();
	const FMVCharacterStatRow* StatRow = TableManager->FindRow<FMVCharacterStatRow>(StatTableName, StatRowKey);
	if (!StatRow)
	{
		return false;
	}

	if (StatRow->CharacterIndexCode != CharacterIndexCode)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("CharacterStat row '%s' has CharacterIndexCode %s, expected %s."),
			*StatRowKey,
			*StatRow->CharacterIndexCode.ToString(),
			*CharacterIndexCode.ToString());
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
	SetAttackPower(StatRow->AttackPower);
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

void UMVStatComponent::HandleDamaged(const FMVResolvedHitData& HitData)
{
	if (bIsDead)
	{
		return;
	}

	if (HitData.VictimCharacterIndexCode.IsValid() && HitData.VictimCharacterIndexCode != CharacterIndexCode)
	{
		return;
	}

	const float HPDamage = MVStatNonNegative(HitData.FinalDamage);
	if (HPDamage > 0.0f)
	{
		PendingDeathHitData = HitData;
		bHasPendingDeathHitData = true;
		SetCurrentHP(CurrentHP - HPDamage);
		bHasPendingDeathHitData = false;
	}

	if (bIsDead)
	{
		return;
	}

	const float GroggyDamage = MVStatNonNegative(HitData.GroggyDamage);
	if (GroggyDamage > 0.0f)
	{
		SetCurrentGroggy(CurrentGroggy + GroggyDamage);
		if (HitData.HitReactionType == EMVActionHitReactionType::Groggy)
		{
			TryStartGroggy();
		}
	}
}

bool UMVStatComponent::WouldDieFromHit(const FMVResolvedHitData& HitData) const
{
	if (bIsDead)
	{
		return true;
	}

	if (HitData.VictimCharacterIndexCode.IsValid() && HitData.VictimCharacterIndexCode != CharacterIndexCode)
	{
		return false;
	}

	const float HPDamage = MVStatNonNegative(HitData.FinalDamage);
	return HPDamage > 0.0f && CurrentHP > 0.0f && CurrentHP - HPDamage <= 0.0f;
}

void UMVStatComponent::TickRecoverableStats(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// MP recovers regardless of whether an action pauses stamina recovery.
	RecoverMP(MPRecoveryPerSecond * DeltaTime);
	TickGroggyRecovery(DeltaTime);

	if (!IsRecoverableStatRecoveryPaused())
	{
		TickRecoverableResourceRecovery(DeltaTime);
	}
}

void UMVStatComponent::TickGroggyRecovery(float DeltaTime)
{
	if (!bIsGroggy)
	{
		return;
	}

	if (GroggyRecoveryCooldownRemaining > 0.0f)
	{
		GroggyRecoveryCooldownRemaining = FMath::Max(0.0f, GroggyRecoveryCooldownRemaining - DeltaTime);
	}

	if (GroggyRecoveryCooldownRemaining <= 0.0f)
	{
		RecoverGroggy(GroggyRecoveryPerSecond * DeltaTime);
	}
}

void UMVStatComponent::TickRecoverableResourceRecovery(float DeltaTime)
{
	if (bUseRecoverableStatRecoveryDelay)
	{
		StaminaCooldownRemaining = FMath::Max(0.0f, StaminaCooldownRemaining - DeltaTime);
	}
	else
	{
		StaminaCooldownRemaining = 0.0f;
	}

	if (StaminaCooldownRemaining <= 0.0f)
	{
		RecoverStamina(StaminaRecoveryPerSecond * DeltaTime);
	}
}

void UMVStatComponent::RestartRecoverableStatCooldown()
{
	StaminaCooldownRemaining = bUseRecoverableStatRecoveryDelay
		? StaminaRecoveryDelay
		: 0.0f;
}

void UMVStatComponent::BeginRecoverableStatRecoveryPause()
{
	const bool bWasPaused = IsRecoverableStatRecoveryPaused();
	++RecoverableStatRecoveryPauseCount;
	if (!bWasPaused)
	{
		StaminaCooldownRemaining = 0.0f;
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

void UMVStatComponent::ResetDeathState()
{
	bIsDead = false;
	bHasPendingDeathHitData = false;
	PendingDeathHitData = FMVResolvedHitData();
}

void UMVStatComponent::ResetGroggyState()
{
	bIsGroggy = false;
	GroggyRecoveryCooldownRemaining = 0.0f;
	SetCurrentGroggy(0.0f);
}

void UMVStatComponent::SetMaxHP(float InMaxHP)
{
	const float PreviousMaxHP = MaxHP;
	const float PreviousCurrentHP = CurrentHP;
	MaxHP = MVStatClampMaxHP(InMaxHP);
	CurrentHP = MVStatClampCurrent(CurrentHP, MaxHP);

	if (!FMath::IsNearlyEqual(PreviousMaxHP, MaxHP) || !FMath::IsNearlyEqual(PreviousCurrentHP, CurrentHP))
	{
		OnHPChanged.Broadcast(CurrentHP, MaxHP);
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
		BroadcastDeathStarted(EMVDeathReason::HPDepleted);
	}
}

void UMVStatComponent::RecoverHP(const float Amount)
{
	const float RecoveryAmount = MVStatNonNegative(Amount);
	if (RecoveryAmount <= 0.0f || bIsDead)
	{
		return;
	}

	SetCurrentHP(CurrentHP + RecoveryAmount);
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
	if(RequiredAmount <= 0.0f)
	{
		return true;
	}
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
	if(RequiredAmount <= 0.0f)
	{
		return true;
	}
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

void UMVStatComponent::SetAttackPower(float InAttackPower)
{
	AttackPower = MVStatNonNegative(InAttackPower);
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
	if (MaxGroggy <= 0.0f || CurrentGroggy < MaxGroggy)
	{
		bIsGroggy = false;
	}

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

	if (bIsGroggy && CurrentGroggy <= 0.0f)
	{
		BroadcastGroggyEnded();
	}
}

void UMVStatComponent::RecoverGroggy(float Amount)
{
	const float NormalizedAmount = MVStatNonNegative(Amount);
	if (NormalizedAmount <= 0.0f)
	{
		return;
	}

	SetCurrentGroggy(CurrentGroggy - NormalizedAmount);
}

void UMVStatComponent::SetGroggyRecoveryPerSecond(float InGroggyRecoveryPerSecond)
{
	GroggyRecoveryPerSecond = MVStatNonNegative(InGroggyRecoveryPerSecond);
}

void UMVStatComponent::SetGroggyRecoveryDelay(float InGroggyRecoveryDelay)
{
	GroggyRecoveryDelay = MVStatNonNegative(InGroggyRecoveryDelay);
}

void UMVStatComponent::RestartGroggyRecoveryCooldown()
{
	GroggyRecoveryCooldownRemaining = GroggyRecoveryDelay;
}

bool UMVStatComponent::TryStartGroggy()
{
	if (bIsGroggy || MaxGroggy <= 0.0f || CurrentGroggy < MaxGroggy)
	{
		return false;
	}

	bIsGroggy = true;
	RestartGroggyRecoveryCooldown();
	OnGroggyStarted.Broadcast();
	return true;
}

FString UMVStatComponent::MakeStatRowKey() const
{
	return CharacterIndexCode.ToString();
}

void UMVStatComponent::BroadcastDeathStarted(const EMVDeathReason Reason)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	FMVDeathContext DeathContext;
	DeathContext.DeadActor = GetOwner();
	DeathContext.Reason = Reason;
	DeathContext.bHasHitData = bHasPendingDeathHitData;
	if (bHasPendingDeathHitData)
	{
		DeathContext.HitData = PendingDeathHitData;
	}

	OnDeathStarted.Broadcast(DeathContext);
	OnDead.Broadcast();
}

void UMVStatComponent::BroadcastGroggyEnded()
{
	if (!bIsGroggy)
	{
		return;
	}

	bIsGroggy = false;
	GroggyRecoveryCooldownRemaining = 0.0f;
	OnGroggyEnded.Broadcast();
}
