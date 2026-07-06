#include "Combat/MVHitResolverSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/World.h"

UMVHitResolverSubsystem* UMVHitResolverSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UMVHitResolverSubsystem>() : nullptr;
}

bool UMVHitResolverSubsystem::ResolveAttackHit(
	const FMVHitResolveRequest& Request,
	FMVResolvedHitData& OutHitData)
{
	if (!BuildResolvedHitData(Request, OutHitData))
	{
		return false;
	}

	OnHitResolved.Broadcast(OutHitData); // VFX, UI 등 후속 처리용 이벤트 브로드캐스트

	if (AMVCharacterBase* Victim = OutHitData.Victim.Get())
	{
		Victim->OnHitResolved(OutHitData); // 피격자에게 피해 처리를 명령하는 곳
	}

	return true;
}

bool UMVHitResolverSubsystem::BuildResolvedHitData(
	const FMVHitResolveRequest& Request,
	FMVResolvedHitData& OutHitData) const
{
	OutHitData = FMVResolvedHitData();

	AMVCharacterBase* Attacker = Request.Attacker.Get();
	AMVCharacterBase* Victim = Request.Victim.Get();
	if (!Attacker || !Victim || Attacker == Victim)
	{
		return false;
	}

	const UMVStatComponent* AttackerStat = Attacker->FindComponentByClass<UMVStatComponent>();
	const UMVStatComponent* VictimStat = Victim->FindComponentByClass<UMVStatComponent>();
	if (!AttackerStat || !VictimStat)
	{
		return false;
	}

	const float AttackerAttackPower = ResolveNonNegativeStat(AttackerStat->AttackPower);
	const float BaseAttackPower = AttackerAttackPower > 0.0f
		? AttackerAttackPower
		: ResolveNonNegativeStat(FallbackAttackPower);
	const FMVWeaponHitSnapshot WeaponSnapshot = ResolveWeaponHitSnapshot(*Attacker, Request);
	const float WeaponAttackPower = ResolveNonNegativeStat(WeaponSnapshot.AttackPower);
	const float DamageMultiplier = ResolveNonNegativeStat(Request.DamageMultiplier);
	const float GroggyDamageMultiplier = ResolveNonNegativeStat(Request.GroggyDamageMultiplier);
	const float VictimDefence = ResolveNonNegativeStat(VictimStat->Defence);
	const float RawDamage = WeaponAttackPower * DamageMultiplier;
	const float FinalDamage = FMath::Max(0.0f, RawDamage - VictimDefence);
	const float GroggyDamage = WeaponAttackPower * GroggyDamageMultiplier;

	OutHitData.Attacker = Attacker;
	OutHitData.Victim = Victim;
	OutHitData.AttackerCharacterIndexCode = Attacker->GetCharacterIndexCode();
	OutHitData.VictimCharacterIndexCode = Victim->GetCharacterIndexCode();
	OutHitData.ActionRowName = Request.ActionRowName;
	OutHitData.ActionTag = Request.ActionTag.IsNone()
		? Request.ActionRowName
		: Request.ActionTag;
	OutHitData.CharacterAttackPower = BaseAttackPower;
	OutHitData.WeaponSnapshot = WeaponSnapshot;
	OutHitData.WeaponAttackPower = WeaponAttackPower;
	OutHitData.VictimDefence = VictimDefence;
	OutHitData.DamageMultiplier = DamageMultiplier;
	OutHitData.GroggyDamageMultiplier = GroggyDamageMultiplier;
	OutHitData.FinalDamage = FinalDamage;
	OutHitData.GroggyDamage = GroggyDamage;
	OutHitData.HitReactionType = Request.HitReactionType;
	OutHitData.HitLocation = Request.HitLocation;
	OutHitData.HitDirection = Request.HitDirection.IsNearlyZero()
		? ResolveHitDirection(*Attacker, *Victim)
		: Request.HitDirection.GetSafeNormal();

	if (const UMVHitReactionComponent* HitReactionComponent = Victim->FindComponentByClass<UMVHitReactionComponent>();
		HitReactionComponent && HitReactionComponent->CanTriggerGroggy(OutHitData))
	{
		OutHitData.HitReactionType = EMVActionHitReactionType::Groggy;
	}

	return true;
}

FMVWeaponHitSnapshot UMVHitResolverSubsystem::ResolveWeaponHitSnapshot(
	const AMVCharacterBase& Attacker,
	const FMVHitResolveRequest& Request) const
{
	if (const UMVWeaponComponent* WeaponComponent = Attacker.FindComponentByClass<UMVWeaponComponent>())
	{
		const FMVWeaponHitSnapshot WeaponSnapshot = WeaponComponent->CaptureWeaponHitSnapshot();
		if (WeaponSnapshot.bValid)
		{
			return WeaponSnapshot;
		}
	}

	FMVWeaponHitSnapshot FallbackSnapshot;
	FallbackSnapshot.AttackPower = Request.WeaponAttackPower > 0.0f
		? ResolveNonNegativeStat(Request.WeaponAttackPower)
		: ResolveNonNegativeStat(FallbackAttackPower);
	FallbackSnapshot.bValid = FallbackSnapshot.AttackPower > 0.0f;
	return FallbackSnapshot;
}

float UMVHitResolverSubsystem::ResolveNonNegativeStat(const float Value)
{
	return FMath::Max(0.0f, Value);
}

FVector UMVHitResolverSubsystem::ResolveHitDirection(
	const AMVCharacterBase& Attacker,
	const AMVCharacterBase& Victim)
{
	const FVector Direction = Victim.GetActorLocation() - Attacker.GetActorLocation();
	return Direction.IsNearlyZero() ? FVector::ZeroVector : Direction.GetSafeNormal();
}
