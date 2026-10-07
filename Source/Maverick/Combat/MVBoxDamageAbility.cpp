#include "Combat/MVBoxDamageAbility.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMVBoxDamageAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	if (bAbilityActive) return;
	AMVCharacterBase* Character = GetOwnerCharacter();
	UWorld* World = IsValid(Character) ? Character->GetWorld() : nullptr;
	if (!World || !World->IsGameWorld() || Character->IsActorBeingDestroyed()
		|| (Character->StatComponent && Character->StatComponent->IsDead())) return;
	Super::StartAbility_Implementation(AbilityIndex);
	if (!bAbilityActive) return;
	HitActors.Reset();
	BoxWorld = World;
	World->GetTimerManager().SetTimer(BoxTimer, this, &UMVBoxDamageAbility::CheckBoxDamage, 1.0f / 60.0f, true);
	CheckBoxDamage();
}

void UMVBoxDamageAbility::ClearBoxTimer()
{
	if (UWorld* World = BoxWorld.Get()) World->GetTimerManager().ClearTimer(BoxTimer);
	BoxWorld.Reset();
}

void UMVBoxDamageAbility::EndAbility_Implementation()
{
	ClearBoxTimer();
	HitActors.Reset();
	Super::EndAbility_Implementation();
}

void UMVBoxDamageAbility::BeginDestroy()
{
	ClearBoxTimer();
	Super::BeginDestroy();
}

void UMVBoxDamageAbility::CheckBoxDamage()
{
	AMVCharacterBase* Character = GetOwnerCharacter();
	UWorld* World = BoxWorld.Get();
	if (!bAbilityActive || !IsValid(Character) || !World || Character->IsActorBeingDestroyed()
		|| (Character->StatComponent && Character->StatComponent->IsDead()))
	{
		EndAbility_Implementation();
		return;
	}
	const FQuat Rotation = Character->GetActorQuat();
	const FVector Center = Character->GetActorLocation() + Rotation.RotateVector(BoxOffset);
	const FVector HalfExtent = BoxSize.ComponentMax(FVector(1.0f)) * 0.5f;
	TArray<FOverlapResult> Overlaps;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(MVBoxDamage), false, Character);
	World->OverlapMultiByObjectType(Overlaps, Center, Rotation,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeBox(HalfExtent), Params);
	UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(World);
	bool bHit = false;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (!bAbilityActive) break;
		AMVCharacterBase* Victim = Cast<AMVCharacterBase>(Overlap.GetActor());
		if (!Resolver || !IsValid(Victim) || Victim == Character || Victim->IsActorBeingDestroyed()
			|| !Victim->IsPlayerControlled() || Victim->IsInvincible() || HitActors.Contains(Victim)
			|| (Victim->StatComponent && Victim->StatComponent->IsDead())) continue;
		FMVHitResolveRequest Request;
		Request.Attacker = Character;
		Request.Victim = Victim;
		Request.AttackInstanceId = GetAttackInstanceId();
		Request.DamageMultiplier = AbilityData.DamageMultiplier;
		Request.GroggyDamageMultiplier = 0.0f;
		Request.HitReactionType = EMVActionHitReactionType::None;
		Request.HitLocation = Victim->GetActorLocation();
		HitActors.Add(Victim);
		FMVResolvedHitData Resolved;
		if (Resolver->ResolveAttackHit(Request, Resolved)) bHit = true;
		else HitActors.Remove(Victim);
	}
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugBox(World, Center, HalfExtent, Rotation, bHit ? FColor::Green : FColor::Yellow, false, 0.05f, 0, 2.0f);
	}
#endif
}
