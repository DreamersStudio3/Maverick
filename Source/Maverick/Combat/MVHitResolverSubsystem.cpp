#include "Combat/MVHitResolverSubsystem.h"

#include "Character/MVCharacterBase.h"
#include "Components/MVHitReactionComponent.h"
#include "Components/MVStatComponent.h"
#include "Components/MVWeaponComponent.h"
#include "Engine/World.h"
#include "MVHitResolverSubsystem.h"

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
	FVector HitDirection = Victim.GetActorLocation() - Attacker.GetActorLocation();
	if (HitDirection.IsNearlyZero())
	{
		// 공격자와 피해자가 같은 위치에 있는 경우, 피격자의 정면 방향의 반대를 사용
		HitDirection = -Victim.GetActorForwardVector();
	}

	return MVHitResolverTryNormalize2D(HitDirection, OutDirection);
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
	if (!BuildResolvedHitData(Request, OutHitData))
	{
		return false;
	}

	return DispatchResolvedHit(OutHitData);
}

bool UMVHitResolverSubsystem::ResolveDirectDamage(const FMVDirectDamageRequest& Request, FMVResolvedHitData& OutHitData)
{
	if (!BuildDirectDamageHitData(Request, OutHitData))
	{
		return false;
	}

	return DispatchResolvedHit(OutHitData);
}

bool UMVHitResolverSubsystem::BuildResolvedHitData(
	const FMVHitResolveRequest& Request,
	FMVResolvedHitData& OutHitData) const
{
	OutHitData = FMVResolvedHitData();

	AMVCharacterBase* Attacker = Request.Attacker.Get();
	AMVCharacterBase* Victim = Request.Victim.Get();

	// Todo: 자기 자신의 체력을 깎는 공격을 허용할지 여부를 결정해야 함
	if (!Attacker || !Victim /*|| Attacker == Victim*/)
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
	const FMVWeaponHitSnapshot WeaponSnapshot = ResolveWeaponHitSnapshot(*Attacker);
	const float WeaponAttackPower = ResolveNonNegativeStat(WeaponSnapshot.AttackPower);
	const float DamageMultiplier = ResolveNonNegativeStat(Request.DamageMultiplier);
	const float GroggyDamageMultiplier = ResolveNonNegativeStat(Request.GroggyDamageMultiplier);
	const float RawDamage = WeaponAttackPower * DamageMultiplier;
	const float FinalDamage = ResolveFinalDamage(Request.AttackTypes,AttackerStat, VictimStat, RawDamage);
	const float GroggyDamage = WeaponAttackPower * GroggyDamageMultiplier;

	OutHitData.Attacker = Attacker;
	OutHitData.Victim = Victim;
	OutHitData.AttackInstanceId = Request.AttackInstanceId;
	OutHitData.AttackerCharacterIndexCode = Attacker->GetCharacterIndexCode();
	OutHitData.VictimCharacterIndexCode = Victim->GetCharacterIndexCode();
	OutHitData.CharacterAttackPower = BaseAttackPower;
	OutHitData.WeaponSnapshot = WeaponSnapshot;
	OutHitData.WeaponAttackPower = WeaponAttackPower;
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
	OutHitData.HitDirection = ResolveHitDirection(*Attacker, *Victim);
	OutHitData.PoiseDamage = ResolveNonNegativeStat(Request.PoiseDamage);
	OutHitData.PoiseBreak = VictimStat->PredictPoiseBreak(Request.PoiseDamage);
	if (VictimStat->IsPoiseBreakGroggyWindowActive())
	{
		// 기존 공격의 GroggyDamage를 강인도 피해로 사용, 명시적인 PoiseDamage 우선
		OutHitData.PoiseDamage = OutHitData.PoiseDamage > 0.0f ? OutHitData.PoiseDamage : GroggyDamage;
		OutHitData.PoiseBreak = VictimStat->PredictPoiseBreak(OutHitData.PoiseDamage);
		OutHitData.HitReactionType = OutHitData.PoiseBreak ? EMVActionHitReactionType::Groggy : EMVActionHitReactionType::None;
		OutHitData.GroggyDamage = OutHitData.PoiseBreak ? FMath::Max(1.0f, VictimStat->MaxGroggy - VictimStat->CurrentGroggy) : 0.0f;
		return true;
	}

	// Todo: Groggy 해결하면서 HitReactionComponent 부분을 삭제, 수정해야함
	const UMVHitReactionComponent* HitReactionComponent = Victim->FindComponentByClass<UMVHitReactionComponent>();
	const bool bCanTriggerGroggy = HitReactionComponent && HitReactionComponent->CanTriggerGroggy(OutHitData);
	if (bCanTriggerGroggy)
	{
		OutHitData.HitReactionType = EMVActionHitReactionType::Groggy;
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
	FVector Direction = FVector::ZeroVector;
	MVHitResolverTryResolveAttackerToVictimDirection(Attacker, Victim, Direction);

	return Direction;
}

bool UMVHitResolverSubsystem::BuildDirectDamageHitData(const FMVDirectDamageRequest& Request,
	FMVResolvedHitData& OutHitData) const
{
	OutHitData = FMVResolvedHitData();

	AMVCharacterBase* Attacker = Request.Attacker.Get();
	AMVCharacterBase* Victim = Request.Victim.Get();

	if (!IsValid(Attacker) || !IsValid(Victim))
	{
		return false;
	}

	const UMVStatComponent* AttackerStatComponent =	Attacker->FindComponentByClass<UMVStatComponent>();

	const UMVStatComponent* VictimStatComponent = Victim->FindComponentByClass<UMVStatComponent>();

	if (!IsValid(AttackerStatComponent) || !IsValid(VictimStatComponent))
	{
		return false;
	}

	if (VictimStatComponent->IsDead())
	{
		return false;
	}

	const float FinalDamage = FMath::Max(0.0f, Request.FinalDamage);
	const float GroggyDamage = FMath::Max(0.0f, Request.GroggyDamage);

	if (FinalDamage <= 0.0f && GroggyDamage <= 0.0f)
	{
		return false;
	}

	OutHitData.Attacker = Attacker;
	OutHitData.Victim = Victim;

	OutHitData.AttackerCharacterIndexCode =	Attacker->GetCharacterIndexCode();
	OutHitData.VictimCharacterIndexCode = Victim->GetCharacterIndexCode();

	OutHitData.AttackInstanceId = Request.AttackInstanceId;

	const float AttackerAttackPower = ResolveNonNegativeStat(AttackerStatComponent->AttackPower);
	const float BaseAttackPower = AttackerAttackPower > 0.0f ? AttackerAttackPower : ResolveNonNegativeStat(FallbackAttackPower);

	OutHitData.CharacterAttackPower = BaseAttackPower;

	OutHitData.DamageMultiplier = 1.0f;
	OutHitData.GroggyDamageMultiplier = 1.0f;

	OutHitData.FinalDamage = FinalDamage;
	OutHitData.GroggyDamage = GroggyDamage;

	OutHitData.HitReactionType = Request.HitReactionType;
	OutHitData.HitLaunchData = Request.HitLaunchData;
	OutHitData.HitLocation = Request.HitLocation;

	OutHitData.ImpactNormal = Request.ImpactNormal.IsNearlyZero() ? FVector::ZeroVector	: Request.ImpactNormal.GetSafeNormal();
	MVHitResolverTryResolveAttackerToVictimDirection(
		*Attacker,
		*Victim,
		OutHitData.HitDirection);

	return true;
}

bool UMVHitResolverSubsystem::DispatchResolvedHit(const FMVResolvedHitData& HitData)
{
	AMVCharacterBase* Victim = HitData.Victim.Get();
	if (!IsValid(Victim))
	{
		return false;
	}

	OnHitResolved.Broadcast(HitData);
	Victim->OnHitResolved(HitData);

	return true;
}

float UMVHitResolverSubsystem::ResolveFinalDamage(const EMVAttackTypes AttackTypes, const UMVStatComponent* AttackerStat, const UMVStatComponent* VictimStat, float RawDamage) const
{
	float VictimDefence = 0.0f;

	// 일반 공격인 경우 기본 방어력 고려
	if (AttackTypes == EMVAttackTypes::NormalAttack)
	{
		VictimDefence = VictimStat->NormalDefence;
	}
	// 스킬 공격인 경우 스킬 방어력 고려
	else if (AttackTypes == EMVAttackTypes::SkillAttack)
	{
		VictimDefence = VictimStat->SkillDefence;
	}

	// 방어력에 기반하여 공격력 계산(소수점 한자리)
	float DamageAmount = FMath::Max(RawDamage - VictimDefence * 0.5f, 0);
	DamageAmount = FMath::TruncToFloat(DamageAmount * 10.0f) / 10.0f;

	float CriticalDamagePercent = AttackerStat->CriticalPercent;
	// 치명타율이 0이면 이전 계산한 데미지 리턴
	if (CriticalDamagePercent <= 0)
	{
		return DamageAmount;
	}

	float RandomSeed = FMath::FRand();
	// 랜덤 Seed가 치명타율보다 크면 이전 계산한 데미지 리턴
	if (RandomSeed > CriticalDamagePercent)
	{
		return DamageAmount;
	}

	// 치명타인 경우 이전 계산한 데미지에 치명타데미지(배율)을 곱해서 리턴 -> 배율은 1이 원래 데미지
	float FinalDamage = DamageAmount * (AttackerStat->CriticalDamage);


	return FinalDamage;
}
