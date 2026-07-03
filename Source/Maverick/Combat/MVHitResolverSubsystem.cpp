#include "Combat/MVHitResolverSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
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
	const float WeaponAttackPower = ResolveEquippedWeaponAttackPower(*Attacker, Request);
	const float DamageMultiplier = ResolveNonNegativeStat(Request.DamageMultiplier);
	const float GroggyDamage = ResolveNonNegativeStat(Request.GroggyDamage);
	const float VictimDefence = ResolveNonNegativeStat(VictimStat->Defence);
	const float RawDamage = (BaseAttackPower + WeaponAttackPower) * DamageMultiplier;
	const float FinalDamage = FMath::Max(0.0f, RawDamage - VictimDefence);

	OutHitData.Attacker = Attacker;
	OutHitData.Victim = Victim;
	OutHitData.AttackerCharacterIndexCode = Attacker->GetCharacterIndexCode();
	OutHitData.VictimCharacterIndexCode = Victim->GetCharacterIndexCode();
	OutHitData.ActionRowName = Request.ActionRowName;
	OutHitData.ActionTag = Request.ActionTag.IsNone()
		? Request.ActionRowName
		: Request.ActionTag;
	OutHitData.CharacterAttackPower = BaseAttackPower;
	OutHitData.WeaponAttackPower = WeaponAttackPower;
	OutHitData.VictimDefence = VictimDefence;
	OutHitData.DamageMultiplier = DamageMultiplier;
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

float UMVHitResolverSubsystem::ResolveEquippedWeaponAttackPower(
	const AMVCharacterBase& /*Attacker*/,
	const FMVHitResolveRequest& Request) const
{
	// WeaponComponent가 편입되면 Attacker의 현재 무기 스탯을 여기서 조회한다.
	// 무기 아이템이 없는 캐릭터도 맨손 무기를 기본 장착한 것으로 취급한다.
	return ResolveNonNegativeStat(Request.WeaponAttackPower);
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
