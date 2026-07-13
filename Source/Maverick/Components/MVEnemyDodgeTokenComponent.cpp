#include "Components/MVEnemyDodgeTokenComponent.h"

#include "Character/MVCharacterBase.h"
#include "Character/NPC/Enemy/MVEnemy.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UMVEnemyDodgeTokenComponent::UMVEnemyDodgeTokenComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMVEnemyDodgeTokenComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemy = Cast<AMVEnemy>(GetOwner());
	BindOwnerEvents();
	BindObservedTargetDamage();
}

void UMVEnemyDodgeTokenComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindObservedTargetDamage();
	UnbindOwnerEvents();
	Super::EndPlay(EndPlayReason);
}

void UMVEnemyDodgeTokenComponent::NotifyEnemyDamaged(const FMVResolvedHitData& HitData)
{
	if (!bGrantOnReceivedHitThreshold || !IsRelevantEnemyHitData(HitData))
	{
		return;
	}

	++ReceivedHitCount;
	if (ReceivedHitThreshold > 0 && ReceivedHitCount >= ReceivedHitThreshold)
	{
		ReceivedHitCount = 0;
		GrantDodgeToken(EMVEnemyDodgeTokenGrantReason::ReceivedHitThreshold);
	}
}

void UMVEnemyDodgeTokenComponent::NotifyEnemyLandedHit(const FMVResolvedHitData& HitData)
{
	if (!bGrantOnLandedHitThreshold || !IsRelevantLandedHitData(HitData))
	{
		return;
	}

	++LandedHitCount;
	if (LandedHitThreshold > 0 && LandedHitCount >= LandedHitThreshold)
	{
		LandedHitCount = 0;
		GrantDodgeToken(EMVEnemyDodgeTokenGrantReason::LandedHitThreshold);
	}
}

void UMVEnemyDodgeTokenComponent::NotifyGroggyEndedByFinisher()
{
	bGroggyEndedByFinisher = true;
}

void UMVEnemyDodgeTokenComponent::GrantDodgeToken(const EMVEnemyDodgeTokenGrantReason Reason)
{
	if (MaxDodgeTokens <= 0)
	{
		return;
	}

	DodgeTokenCount = FMath::Clamp(DodgeTokenCount + 1, 0, MaxDodgeTokens);
	LastGrantReason = Reason;
}

bool UMVEnemyDodgeTokenComponent::CanSpendDodgeToken() const
{
	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
	return DodgeTokenCount > 0 && CurrentTime >= NextSpendAllowedTime;
}

bool UMVEnemyDodgeTokenComponent::TrySpendDodgeToken()
{
	if (!CanSpendDodgeToken())
	{
		return false;
	}

	--DodgeTokenCount;
	if (UWorld* World = GetWorld())
	{
		NextSpendAllowedTime = World->GetTimeSeconds() + FMath::Max(0.0f, SpendCooldown);
	}

	ResetCombatFlowCounters();

	return true;
}

void UMVEnemyDodgeTokenComponent::ResetForFieldTransition()
{
	DodgeTokenCount = 0;
	LastGrantReason = EMVEnemyDodgeTokenGrantReason::None;
	NextSpendAllowedTime = 0.0f;
	bGroggyActive = false;
	bGroggyEndedByFinisher = false;
	ResetCombatFlowCounters();
}

bool UMVEnemyDodgeTokenComponent::TryConsumeDodgeTokenForThreat(
	AActor* Target,
	const FMVCombatActionEvent& CombatActionEvent,
	const FMVDodgeThreatConfig& ThreatConfig,
	const bool bOwnerActionRunning,
	FMVAIDodgeRequest& OutDodgeRequest,
	FMVDodgeThreatDecision& OutThreatDecision)
{
	OutDodgeRequest = FMVAIDodgeRequest();
	OutThreatDecision = UMVDodgeThreatEvaluatorLibrary::EvaluateDodgeThreat(
		GetOwner(),
		Target,
		CombatActionEvent,
		ThreatConfig,
		bOwnerActionRunning);

	if (!OutThreatDecision.bShouldSendEvent)
	{
		return false;
	}

	if (!TrySpendDodgeToken())
	{
		OutThreatDecision.bShouldSendEvent = false;
		OutThreatDecision.RejectReason = EMVDodgeThreatRejectReason::TokenUnavailable;
		return false;
	}

	OutDodgeRequest = OutThreatDecision.DodgeRequest;
	return true;
}

void UMVEnemyDodgeTokenComponent::BindOwnerEvents()
{
	if (!OwnerEnemy)
	{
		return;
	}

	OwnerEnemy->OnEnemyDamaged.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerDamaged);
	OwnerEnemy->OnEnemyDamaged.AddUniqueDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerDamaged);
	OwnerEnemy->OnEnemyGroggyStarted.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerGroggyStarted);
	OwnerEnemy->OnEnemyGroggyStarted.AddUniqueDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerGroggyStarted);
	OwnerEnemy->OnEnemyGroggyEnded.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerGroggyEnded);
	OwnerEnemy->OnEnemyGroggyEnded.AddUniqueDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerGroggyEnded);
}

void UMVEnemyDodgeTokenComponent::UnbindOwnerEvents()
{
	if (!OwnerEnemy)
	{
		return;
	}

	OwnerEnemy->OnEnemyDamaged.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerDamaged);
	OwnerEnemy->OnEnemyGroggyStarted.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerGroggyStarted);
	OwnerEnemy->OnEnemyGroggyEnded.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleOwnerGroggyEnded);
}

void UMVEnemyDodgeTokenComponent::BindObservedTargetDamage()
{
	if (!bAutoBindPlayerDamage)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	ObservedTargetCharacter = Cast<AMVCharacterBase>(PlayerPawn);
	if (!ObservedTargetCharacter)
	{
		return;
	}

	ObservedTargetCharacter->OnDamaged.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleObservedTargetDamaged);
	ObservedTargetCharacter->OnDamaged.AddUniqueDynamic(this, &UMVEnemyDodgeTokenComponent::HandleObservedTargetDamaged);
}

void UMVEnemyDodgeTokenComponent::UnbindObservedTargetDamage()
{
	if (!ObservedTargetCharacter)
	{
		return;
	}

	ObservedTargetCharacter->OnDamaged.RemoveDynamic(this, &UMVEnemyDodgeTokenComponent::HandleObservedTargetDamaged);
	ObservedTargetCharacter = nullptr;
}

void UMVEnemyDodgeTokenComponent::ResetCombatFlowCounters()
{
	ReceivedHitCount = 0;
	LandedHitCount = 0;
}

bool UMVEnemyDodgeTokenComponent::IsRelevantEnemyHitData(const FMVResolvedHitData& HitData) const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor
		&& HitData.Victim.Get() == OwnerActor
		&& HitData.Attacker.Get() != OwnerActor
		&& HitData.FinalDamage > 0.0f;
}

bool UMVEnemyDodgeTokenComponent::IsRelevantLandedHitData(const FMVResolvedHitData& HitData) const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor
		&& HitData.Attacker.Get() == OwnerActor
		&& HitData.Victim.Get() != OwnerActor
		&& HitData.FinalDamage > 0.0f;
}

void UMVEnemyDodgeTokenComponent::HandleOwnerDamaged(const FMVResolvedHitData& HitData)
{
	NotifyEnemyDamaged(HitData);
}

void UMVEnemyDodgeTokenComponent::HandleOwnerGroggyStarted()
{
	bGroggyActive = true;
	bGroggyEndedByFinisher = false;
}

void UMVEnemyDodgeTokenComponent::HandleOwnerGroggyEnded()
{
	if (bGrantOnGroggyRecovered && bGroggyActive && !bGroggyEndedByFinisher)
	{
		GrantDodgeToken(EMVEnemyDodgeTokenGrantReason::GroggyRecovered);
	}

	bGroggyActive = false;
	bGroggyEndedByFinisher = false;
}

void UMVEnemyDodgeTokenComponent::HandleObservedTargetDamaged(const FMVResolvedHitData& HitData)
{
	NotifyEnemyLandedHit(HitData);
}
