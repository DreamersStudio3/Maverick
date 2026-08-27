#include "Combat/MVHitResolverSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/World.h"

namespace
{
bool MVHitResolverTryNormalize2D(const FVector& Direction, FVector& OutDirection)
{
	const FVector Direction2D(Direction.X, Direction.Y, 0.0f);
	if (Direction2D.IsNearlyZero())
	{
		return false;
	}

	OutDirection = Direction2D.GetSafeNormal2D();
	return true;
}

bool MVHitResolverTryResolveAttackerToVictimDirection(
	const AMVCharacterBase& Attacker,
	const AMVCharacterBase& Victim,
	FVector& OutDirection)
{
	return MVHitResolverTryNormalize2D(Victim.GetActorLocation() - Attacker.GetActorLocation(), OutDirection);
}

void MVHitResolverLogHitLaunchTrace(
	const TCHAR* Stage,
	const FMVHitResolveRequest& Request,
	const FMVResolvedHitData* ResolvedHitData = nullptr)
{
	const FMVHitLaunchData& LaunchData = ResolvedHitData
		? ResolvedHitData->HitLaunchData
		: Request.HitLaunchData;
	const FString HitDirectionText = ResolvedHitData
		? ResolvedHitData->HitDirection.ToString()
		: TEXT("<pending>");
	UE_LOG(
		LogTemp,
		Log,
		TEXT("HitLaunchTrace Frame=%llu Stage=%s Attacker=%s Victim=%s HitReactionType=%d Distance=%.2f Duration=%.3f VerticalSpeed=%.2f HitLocation=%s ImpactNormal=%s HitDirection=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		Stage,
		*GetNameSafe(Request.Attacker.Get()),
		*GetNameSafe(Request.Victim.Get()),
		static_cast<int32>(ResolvedHitData ? ResolvedHitData->HitReactionType : Request.HitReactionType),
		LaunchData.LaunchDistance,
		LaunchData.LaunchDuration,
		LaunchData.LaunchVerticalSpeed,
		*(ResolvedHitData ? ResolvedHitData->HitLocation.ToString() : Request.HitLocation.ToString()),
		*(ResolvedHitData ? ResolvedHitData->ImpactNormal.ToString() : Request.ImpactNormal.ToString()),
		*HitDirectionText);
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
	const FString HitDirectionText = ResolvedHitData
		? ResolvedHitData->HitDirection.ToString()
		: TEXT("<pending>");
	const FVector& HitLocation = ResolvedHitData
		? ResolvedHitData->HitLocation
		: Request.HitLocation;
	const FVector& ImpactNormal = ResolvedHitData
		? ResolvedHitData->ImpactNormal
		: Request.ImpactNormal;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("AirborneTrace Frame=%llu Stage=%s Attacker=%s Victim=%s bAttackerValid=%s bVictimValid=%s bSelfHit=%s RequestType=%d ResolvedType=%d Distance=%.2f Duration=%.3f VerticalSpeed=%.2f HitLocation=%s ImpactNormal=%s HitDirection=%s"),
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
		*HitLocation.ToString(),
		*ImpactNormal.ToString(),
		*HitDirectionText);
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
	if (!Attacker || !Victim /*|| Attacker == Victim*/)
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
	OutHitData.ImpactNormal = Request.ImpactNormal.IsNearlyZero()
		? FVector::ZeroVector
		: Request.ImpactNormal.GetSafeNormal();
	OutHitData.HitDirection = ResolveHitDirection(Request, *Attacker, *Victim);
	OutHitData.PoiseDamage = ResolveNonNegativeStat(Request.PoiseDamage);
	OutHitData.PoiseBreak = VictimStat->PredictPoiseBreak(Request.PoiseDamage);

	const UMVHitReactionComponent* HitReactionComponent = Victim->FindComponentByClass<UMVHitReactionComponent>();
	const bool bCanTriggerGroggy = HitReactionComponent && HitReactionComponent->CanTriggerGroggy(OutHitData);
	if (bCanTriggerGroggy)
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
	const FMVHitResolveRequest& Request,
	const AMVCharacterBase& Attacker,
	const AMVCharacterBase& Victim)
{
	FVector Direction = FVector::ZeroVector;
	const bool bHasAttackerToVictimDirection = MVHitResolverTryResolveAttackerToVictimDirection(Attacker, Victim, Direction);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("HitDirectionTrace Frame=%llu Stage=ResolverResolveDirection Source=%s Attacker=%s Victim=%s AttackerLocation=%s VictimLocation=%s HitLocation=%s ImpactNormal=%s Result=%s"),
		static_cast<unsigned long long>(GFrameCounter),
		bHasAttackerToVictimDirection ? TEXT("AttackerToVictim") : TEXT("None"),
		*GetNameSafe(&Attacker),
		*GetNameSafe(&Victim),
		*Attacker.GetActorLocation().ToString(),
		*Victim.GetActorLocation().ToString(),
		*Request.HitLocation.ToString(),
		*Request.ImpactNormal.ToString(),
		*Direction.ToString());
	return Direction;
}
