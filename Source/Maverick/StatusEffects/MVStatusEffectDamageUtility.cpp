#include "StatusEffects/MVStatusEffectDamageUtility.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/MVStatComponent.h"

bool MVStatusEffectDamage::ApplyTargetMaxHealthDamage(
	const UObject* WorldContextObject,
	AActor* SourceActor,
	AActor* TargetActor,
	const float DamageRatio,
	FMVResolvedHitData* OutHitData)
{
	if (OutHitData)
	{
		*OutHitData = FMVResolvedHitData();
	}

	AMVCharacterBase* Attacker = Cast<AMVCharacterBase>(SourceActor);
	AMVCharacterBase* Victim = Cast<AMVCharacterBase>(TargetActor);

	if (!IsValid(Attacker) || !IsValid(Victim))
	{
		return false;
	}

	const UMVStatComponent* VictimStatComponent = Victim->FindComponentByClass<UMVStatComponent>();

	if (!IsValid(VictimStatComponent) || VictimStatComponent->IsDead())
	{
		return false;
	}

	const float SafeDamageRatio = FMath::Max(0.0f, DamageRatio);
	const float TargetMaxHealth = FMath::Max(0.0f, VictimStatComponent->MaxHP);
	const float FinalDamage = TargetMaxHealth * SafeDamageRatio;

	if (FinalDamage <= 0.0f)
	{
		return false;
	}

	UMVHitResolverSubsystem* HitResolver = UMVHitResolverSubsystem::Get(WorldContextObject);

	if (!IsValid(HitResolver))
	{
		return false;
	}

	FMVDirectDamageRequest DamageRequest;
	DamageRequest.Attacker = Attacker;
	DamageRequest.Victim = Victim;
	DamageRequest.AttackInstanceId = INDEX_NONE;
	DamageRequest.FinalDamage = FinalDamage;
	DamageRequest.GroggyDamage = 0.0f;
	DamageRequest.HitReactionType = EMVActionHitReactionType::None;
	DamageRequest.HitLocation = Victim->GetActorLocation();

	FMVResolvedHitData LocalHitData;
	FMVResolvedHitData& ResolvedHitData = OutHitData ? *OutHitData : LocalHitData;

	return HitResolver->ResolveDirectDamage(DamageRequest, ResolvedHitData);
}