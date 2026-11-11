#include "Combat/Boss/ElectricEelMan/MVElectricEelManAbility.h"
#include "Combat/Boss/ElectricEelMan/MVElectricEelManSkillComponent.h"
#include "Character/MVCharacterBase.h"
#include "AI/Controller/MVAIController.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVStatComponent.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

bool UMVElectricEelManAbility::BeginSkill(int32 AbilityIndex, bool bUltimate)
{
	auto* Character = GetOwnerCharacter();
	if (bAbilityActive || bAbilityCostConsumed || !IsValid(Character) || Character->IsActorBeingDestroyed()
		|| !Character->GetWorld()->IsGameWorld() || !Character->StatComponent || Character->StatComponent->IsDead()) return false;
	auto* State = UMVElectricEelManSkillComponent::GetOrCreate(Character);
	if (!State || (bUltimate && !State->CanUseR())) return false;
	Super::StartAbility_Implementation(AbilityIndex);
	if (!bAbilityActive) return false;
	if (bUltimate) State->ConsumeR();
	else State->RecordSkillUse();
	SkillWorld = Character->GetWorld();
	StartTime = SkillWorld->GetTimeSeconds();
	SkillWorld->GetTimerManager().SetTimer(SkillTimer, this, &ThisClass::UpdateSkill, 1.0f / 60.0f, true);
	return true;
}
AMVCharacterBase* UMVElectricEelManAbility::FindTarget()
{
	auto* Character = GetOwnerCharacter();
	if (!IsValid(Character)) return nullptr;
	auto* AI = Cast<AMVAIController>(Character->GetController());
	auto* Target = Cast<AMVCharacterBase>(AI ? AI->TargetActor : UGameplayStatics::GetPlayerCharacter(Character, 0));
	return IsEligiblePlayer(Target) ? Target : nullptr;
}
bool UMVElectricEelManAbility::IsEligiblePlayer(AMVCharacterBase* Player) const
{
	return IsValid(Player) && !Player->IsActorBeingDestroyed() && Player->IsPlayerControlled()
		&& Player->StatComponent && !Player->StatComponent->IsDead() && !Player->IsInvincible();
}
bool UMVElectricEelManAbility::HitPlayer(AMVCharacterBase* Player, float Multiplier, EMVActionHitReactionType Reaction)
{
	if (!bAbilityActive || !IsEligiblePlayer(Player)) return false;
	auto* Resolver = UMVHitResolverSubsystem::Get(Player);
	if (!Resolver) return false;
	FMVHitResolveRequest Request;
	Request.Attacker = GetOwnerCharacter();
	Request.Victim = Player;
	Request.AttackInstanceId = GetAttackInstanceId();
	Request.DamageMultiplier = AbilityData.DamageMultiplier * Multiplier;
	Request.GroggyDamageMultiplier = AbilityData.GroggyDamageMultiplier * Multiplier;
	Request.HitReactionType = Reaction;
	Request.HitLocation = Player->GetActorLocation();
	Request.HitLaunchData = GetHitLaunchData();
	FMVResolvedHitData Resolved;
	return Resolver->ResolveAttackHit(Request, Resolved);
}
void UMVElectricEelManAbility::HitSphere(const FVector& Center, float Radius, float Multiplier,
	EMVActionHitReactionType Reaction, TSet<TWeakObjectPtr<AActor>>& HitActors)
{
	auto* Character = GetOwnerCharacter();
	if (!bAbilityActive || !IsValid(Character)) return;
	TArray<FOverlapResult> Overlaps;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(EelSkillSphere), false, Character);
	Character->GetWorld()->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(FMath::Max(1.0f, Radius)), Params);
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (!bAbilityActive) break;
		auto* Player = Cast<AMVCharacterBase>(Overlap.GetActor());
		if (!IsEligiblePlayer(Player) || Player == Character || HitActors.Contains(Player)) continue;
		HitActors.Add(Player);
		if (!HitPlayer(Player, Multiplier, Reaction)) HitActors.Remove(Player);
	}
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug) DrawDebugSphere(Character->GetWorld(), Center, Radius, 24, FColor::Cyan, false, 0.3f);
#endif
}
void UMVElectricEelManAbility::LockOwnerMovement()
{
	auto* Character = GetOwnerCharacter();
	auto* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement || LockedMovement.IsValid()) return;
	if (auto* AI = Cast<AAIController>(Character->GetController())) AI->StopMovement();
	LockedMovement = Movement;
	SavedMode = Movement->MovementMode;
	SavedCustomMode = Movement->CustomMovementMode;
	Movement->StopMovementImmediately();
	Movement->DisableMovement();
}
void UMVElectricEelManAbility::UpdateSkill()
{
	auto* Character = GetOwnerCharacter();
	if (!bAbilityActive || !IsValid(Character) || Character->IsActorBeingDestroyed()
		|| !SkillWorld.IsValid() || !Character->StatComponent || Character->StatComponent->IsDead())
	{
		EndAbility_Implementation();
		return;
	}
	TickSkill(SkillWorld->GetTimeSeconds() - StartTime);
}
void UMVElectricEelManAbility::StopExecution()
{
	if (auto* World = SkillWorld.Get()) World->GetTimerManager().ClearTimer(SkillTimer);
	SkillWorld.Reset();
	auto* Character = GetOwnerCharacter();
	if (LockedMovement.IsValid() && IsValid(Character) && !Character->IsActorBeingDestroyed()
		&& Character->StatComponent && !Character->StatComponent->IsDead())
		LockedMovement->SetMovementMode(SavedMode, SavedCustomMode);
	LockedMovement.Reset();
}
void UMVElectricEelManAbility::EndAbility_Implementation()
{
	StopExecution();
	Super::EndAbility_Implementation();
}
void UMVElectricEelManAbility::BeginDestroy()
{
	StopExecution();
	Super::BeginDestroy();
}
