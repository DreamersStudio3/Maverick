#include "Components/MVStatComponent.h"

#include "Tables/MVTableManager.h"
#include "Tables/MVStatTableTypes.h"
#include "Tags/MVGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	SetRecentDamageResetDelay(StatRow->GetRecentDamageResetDelay());
	SetInitialPoise(StatRow->InitialCharacterPoise);

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
		const float PreviousHP = CurrentHP;
		PendingDeathHitData = HitData;
		bHasPendingDeathHitData = true;
		SetCurrentHP(CurrentHP - HPDamage);
		bHasPendingDeathHitData = false;

		const float AppliedDamage = FMath::Max(0.0f, PreviousHP - CurrentHP);
		if (AppliedDamage > 0.0f)
		{
			AccumulatedRecentDamage += AppliedDamage;
			bHasRecentDamageAccumulation = true;
			RestartRecentDamageCooldown();
			OnDamageApplied.Broadcast(AppliedDamage, PreviousHP, CurrentHP, HitData);
			OnDamageAccumulated.Broadcast(AccumulatedRecentDamage, AppliedDamage, PreviousHP, CurrentHP, HitData);
			if (RecentDamageResetDelay <= 0.0f)
			{
				ResetDamageAccumulation();
			}
		}
	}

	const float PoiseDamage = MVStatNonNegative(HitData.PoiseDamage);
	if (PoiseDamage > 0.0f)
	{
		UpdatePoise(MVStatNonNegative(HitData.PoiseDamage));
	}

	if (bIsDead)
	{
		return;
	}

	if (bIsGroggy)
	{
		return;
	}

	const float GroggyDamage = MVStatNonNegative(HitData.GroggyDamage);
	if (GroggyDamage > 0.0f)
	{
		SetCurrentGroggy(CurrentGroggy + GroggyDamage);
		RestartRecentDamageCooldown();

		if (HitData.HitReactionType == EMVActionHitReactionType::Groggy)
		{
			TryStartGroggy();
			ResetPoise();
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
	TickRecentDamageCooldown(DeltaTime);
	TickGroggyRecovery(DeltaTime);

	if (!IsRecoverableStatRecoveryPaused())
	{
		TickRecoverableResourceRecovery(DeltaTime);
	}
}

void UMVStatComponent::TickRecentDamageCooldown(float DeltaTime)
{
	if (RecentDamageCooldownRemaining <= 0.0f)
	{
		return;
	}

	RecentDamageCooldownRemaining = FMath::Max(0.0f, RecentDamageCooldownRemaining - DeltaTime);
	if (RecentDamageCooldownRemaining <= 0.0f && bHasRecentDamageAccumulation)
	{
		ResetDamageAccumulation();
	}
}

void UMVStatComponent::TickGroggyRecovery(float DeltaTime)
{
	if (CurrentGroggy <= 0.0f)
	{
		return;
	}

	if (bIsGroggy)
	{
		RecoverGroggy(GroggyRecoveryPerSecond * DeltaTime);
		return;
	}

	if (MaxGroggy > 0.0f && CurrentGroggy >= MaxGroggy)
	{
		return;
	}

	if (RecentDamageCooldownRemaining <= 0.0f)
	{
		RecoverGroggy(GroggyRecoveryPerSecond * DeltaTime);
	}
}

void UMVStatComponent::TickRecoverableResourceRecovery(float DeltaTime)
{
	RecoverStamina(StaminaRecoveryPerSecond * DeltaTime);
}

void UMVStatComponent::BeginRecoverableStatRecoveryPause()
{
	const bool bWasPaused = IsRecoverableStatRecoveryPaused();
	++RecoverableStatRecoveryPauseCount;
	if (!bWasPaused)
	{
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
	RecentDamageCooldownRemaining = 0.0f;
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

bool UMVStatComponent::HasStamina(float RequiredAmount) const
{
	if(RequiredAmount <= 0.0f)
	{
		return true;
	}
	return CurrentStamina >= MVStatNonNegative(RequiredAmount);
}

bool UMVStatComponent::HasAnyStamina() const
{
	return CurrentStamina > 0.0f;
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
	return bHadEnoughStamina;
}

bool UMVStatComponent::ConsumeStaminaAllowPartial(float Amount)
{
	const float NormalizedAmount = MVStatNonNegative(Amount);
	if (NormalizedAmount <= 0.0f)
	{
		return true;
	}

	if (!HasAnyStamina())
	{
		return false;
	}

	SetCurrentStamina(CurrentStamina - FMath::Min(CurrentStamina, NormalizedAmount));
	return true;
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

bool UMVStatComponent::HasReachedSprintSpeedRatio(const float RequiredRatio) const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* MovementComponent = OwnerCharacter
		? OwnerCharacter->GetCharacterMovement()
		: nullptr;
	if (!MovementComponent || SprintSpeed <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float CurrentSpeed = MovementComponent->Velocity.Size2D();
	const float RequiredSpeed = SprintSpeed * FMath::Clamp(RequiredRatio, 0.0f, 1.0f);
	return CurrentSpeed >= RequiredSpeed;
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

void UMVStatComponent::SetRecentDamageResetDelay(float InRecentDamageResetDelay)
{
	RecentDamageResetDelay = MVStatNonNegative(InRecentDamageResetDelay);
}

void UMVStatComponent::SetGroggyRecoveryDelay(float InGroggyRecoveryDelay)
{
	SetRecentDamageResetDelay(InGroggyRecoveryDelay);
}

void UMVStatComponent::SetInitialPoise(float InitialPoise)
{
	InitialCharacterPoise = InitialPoise;
	ConstantPoise = InitialCharacterPoise;
}

void UMVStatComponent::PoiseActionStart(float WeaponPoise, float Multiplier)
{
	SetAdditionalPoise(WeaponPoise, Multiplier);
	AdjustMinPoise();
}

void UMVStatComponent::PoiseActionEnd()
{
	SetAdditionalPoise();
}

void UMVStatComponent::UpdatePoise(float PoiseDamageAmount)
{
	// 피격 등으로 인해 포이즈가 감소하는 경우, ConstantPoise를 감소시킵니다.

	// 강인도 감소량이 0 이하인 경우, 아무 작업도 수행하지 않습니다.
	if(PoiseDamageAmount <= 0.0f)
	{
		return;
	}

	// Poise Reset Timer 새로 시작 -> 피격 등으로 강인도 변화가 생기면 해당 시간 기준으로 일정 시간 이후 강인도 초기화
	UWorld* World = GetWorld();
	if (World)
	{
		if(PoiseRecoveryTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(PoiseRecoveryTimerHandle);
		}
		World->GetTimerManager().SetTimer(PoiseRecoveryTimerHandle, this, &UMVStatComponent::ResetPoise, PoiseRecoveryTime, false);
	}

	ConstantPoise = ConstantPoise - PoiseDamageAmount;
	if(ConstantPoise + AdditionalPoise <= 0.0f)
	{
		// 포이즈가 0 이하로 떨어진 경우, 강인도를 초기화, HitReaction을 Play
		// HitReaction Play를 위한 flag는 HitResolverSubsystem에서 처리하도록 함
		ResetPoise();
	}

}

void UMVStatComponent::ResetPoise()
{
	UWorld* World = GetWorld();
	if (World && PoiseRecoveryTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(PoiseRecoveryTimerHandle);
	}

	if(ConstantPoise < InitialCharacterPoise)
	{
		ConstantPoise = InitialCharacterPoise;
	}
}

bool UMVStatComponent::PredictPoiseBreak(float PoiseDamageAmount) const
{
	if(PoiseDamageAmount <= 0.0f)
	{
		return false;
	}

	float PredictedPoise = ConstantPoise + AdditionalPoise - PoiseDamageAmount;
	bool Result = PredictedPoise <= 0.0f;

	return Result;
}

void UMVStatComponent::RestartRecentDamageCooldown()
{
	RecentDamageCooldownRemaining = RecentDamageResetDelay;
}

void UMVStatComponent::ResetDamageAccumulation()
{
	if (!bHasRecentDamageAccumulation && AccumulatedRecentDamage <= 0.0f)
	{
		return;
	}

	AccumulatedRecentDamage = 0.0f;
	bHasRecentDamageAccumulation = false;
	OnDamageAccumulationReset.Broadcast();
}

bool UMVStatComponent::TryStartGroggy()
{
	if (bIsGroggy || MaxGroggy <= 0.0f || CurrentGroggy < MaxGroggy)
	{
		return false;
	}

	bIsGroggy = true;
	RestartRecentDamageCooldown();
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
	RecentDamageCooldownRemaining = 0.0f;
	OnGroggyEnded.Broadcast();
}

void UMVStatComponent::SetAdditionalPoise(float WeaponPoise, float Multiplier)
{
	if(WeaponPoise <= 0.0f || Multiplier <= 0.0f)
	{
		AdditionalPoise = 0.0f;
		return;
	}

	AdditionalPoise = WeaponPoise * Multiplier;
}

void UMVStatComponent::AdjustMinPoise()
{
	float currentPoise = ConstantPoise + AdditionalPoise;
	float comparePoise = (InitialCharacterPoise + AdditionalPoise) * MinPoiseRecoveryRatio;

	if (currentPoise < comparePoise)
	{
		ConstantPoise = comparePoise - AdditionalPoise;
	}
}

