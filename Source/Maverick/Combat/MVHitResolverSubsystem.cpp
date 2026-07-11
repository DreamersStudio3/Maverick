#include "Combat/MVHitResolverSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/World.h"

namespace
{
void MVHitResolverLogHitLaunchTrace(
	const TCHAR* Stage,
	const FMVHitResolveRequest& Request,
	const FMVResolvedHitData* ResolvedHitData = nullptr)
{
	const FMVHitLaunchData& LaunchData = ResolvedHitData
		? ResolvedHitData->HitLaunchData
		: Request.HitLaunchData;
	UE_LOG(
		LogTemp,
		Log,
		TEXT("HitLaunchTrace Frame=%llu Stage=%s Attacker=%s Victim=%s HitReactionType=%d Distance=%.2f Duration=%.3f VerticalSpeed=%.2f HitDirection=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Request.Attacker.Get()),
		*GetNameSafe(Request.Victim.Get()),
		static_cast<int32>(ResolvedHitData ? ResolvedHitData->HitReactionType : Request.HitReactionType),
		LaunchData.LaunchDistance,
		LaunchData.LaunchDuration,
		LaunchData.LaunchVerticalSpeed,
		*(ResolvedHitData ? ResolvedHitData->HitDirection.ToString() : Request.HitDirection.ToString()));
}

bool MVHitResolverShouldLogAirborneTrace(
	const FMVHitResolveRequest& Request,
	const FMVResolvedHitData* ResolvedHitData = nullptr)
{
	return Request.HitReactionType == EMVActionHitReactionType::Airborne
		|| (ResolvedHitData && ResolvedHitData->HitReactionType == EMVActionHitReactionType::Airborne);
}

void MVHitResolverLogAirborneTrace(
	const TCHAR* Stage,
	const FMVHitResolveRequest& Request,
	const FMVResolvedHitData* ResolvedHitData = nullptr)
{
	if (!MVHitResolverShouldLogAirborneTrace(Request, ResolvedHitData))
	{
		return;
	}

	const FMVHitLaunchData& LaunchData = ResolvedHitData
		? ResolvedHitData->HitLaunchData
		: Request.HitLaunchData;
	const AMVCharacterBase* Attacker = Request.Attacker.Get();
	const AMVCharacterBase* Victim = Request.Victim.Get();
	const FVector& HitDirection = ResolvedHitData
		? ResolvedHitData->HitDirection
		: Request.HitDirection;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("AirborneTrace Frame=%llu Stage=%s Attacker=%s Victim=%s bAttackerValid=%s bVictimValid=%s bSelfHit=%s RequestType=%d ResolvedType=%d Distance=%.2f Duration=%.3f VerticalSpeed=%.2f HitDirection=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Attacker),
		*GetNameSafe(Victim),
		Attacker ? TEXT("true") : TEXT("false"),
		Victim ? TEXT("true") : TEXT("false"),
		(Attacker && Victim && Attacker == Victim) ? TEXT("true") : TEXT("false"),
		static_cast<int32>(Request.HitReactionType),
		static_cast<int32>(ResolvedHitData ? ResolvedHitData->HitReactionType : EMVActionHitReactionType::None),
		LaunchData.LaunchDistance,
		LaunchData.LaunchDuration,
		LaunchData.LaunchVerticalSpeed,
		*HitDirection.ToString());
}
}

UMVHitResolverSubsystem* UMVHitResolverSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UMVHitResolverSubsystem>() : nullptr;
}

bool UMVHitResolverSubsystem::ResolveAttackHit(
	const FMVHitResolveRequest& Request,
	FMVResolvedHitData& OutHitData)
{
	MVHitResolverLogHitLaunchTrace(TEXT("ResolverRequest"), Request);
	MVHitResolverLogAirborneTrace(TEXT("ResolverRequest"), Request);

	if (!BuildResolvedHitData(Request, OutHitData))
	{
		MVHitResolverLogAirborneTrace(TEXT("ResolverRejected"), Request);
		return false;
	}

	MVHitResolverLogHitLaunchTrace(TEXT("ResolverResolved"), Request, &OutHitData);
	MVHitResolverLogAirborneTrace(TEXT("ResolverResolved"), Request, &OutHitData);

	OnHitResolved.Broadcast(OutHitData); // VFX, UI 등 후속 처리용 이벤트 브로드캐스트

	if (AMVCharacterBase* Victim = OutHitData.Victim.Get())
	{
		MVHitResolverLogAirborneTrace(TEXT("ResolverDispatchVictim"), Request, &OutHitData);
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
		MVHitResolverLogAirborneTrace(TEXT("BuildRejected_InvalidParticipants"), Request);
		return false;
	}

	const UMVStatComponent* AttackerStat = Attacker->FindComponentByClass<UMVStatComponent>();
	const UMVStatComponent* VictimStat = Victim->FindComponentByClass<UMVStatComponent>();
	if (!AttackerStat || !VictimStat)
	{
		MVHitResolverLogAirborneTrace(TEXT("BuildRejected_MissingStat"), Request);
		return false;
	}

	const float AttackerAttackPower = ResolveNonNegativeStat(AttackerStat->AttackPower);
	const float BaseAttackPower = AttackerAttackPower > 0.0f
		? AttackerAttackPower
		: ResolveNonNegativeStat(FallbackAttackPower);
	const FMVWeaponHitSnapshot WeaponSnapshot = ResolveWeaponHitSnapshot(*Attacker);
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
	OutHitData.CharacterAttackPower = BaseAttackPower;
	OutHitData.WeaponSnapshot = WeaponSnapshot;
	OutHitData.WeaponAttackPower = WeaponAttackPower;
	OutHitData.VictimDefence = VictimDefence;
	OutHitData.DamageMultiplier = DamageMultiplier;
	OutHitData.GroggyDamageMultiplier = GroggyDamageMultiplier;
	OutHitData.FinalDamage = FinalDamage;
	OutHitData.GroggyDamage = GroggyDamage;
	OutHitData.HitReactionType = Request.HitReactionType;
	OutHitData.HitLaunchData = Request.HitLaunchData;
	OutHitData.HitLocation = Request.HitLocation;
	OutHitData.HitDirection = Request.HitDirection.IsNearlyZero()
		? ResolveHitDirection(*Attacker, *Victim)
		: Request.HitDirection.GetSafeNormal();

	if (const UMVHitReactionComponent* HitReactionComponent = Victim->FindComponentByClass<UMVHitReactionComponent>();
		HitReactionComponent && HitReactionComponent->CanTriggerGroggy(OutHitData))
	{
		OutHitData.HitReactionType = EMVActionHitReactionType::Groggy;
		MVHitResolverLogAirborneTrace(TEXT("BuildConvertedToGroggy"), Request, &OutHitData);
	}

	return true;
}

FMVWeaponHitSnapshot UMVHitResolverSubsystem::ResolveWeaponHitSnapshot(const AMVCharacterBase& Attacker) const
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
	FallbackSnapshot.AttackPower = ResolveNonNegativeStat(FallbackAttackPower);
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
