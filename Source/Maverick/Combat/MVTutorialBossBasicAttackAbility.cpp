#include "Combat/MVTutorialBossBasicAttackAbility.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVHitResolverSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/MVStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMVTutorialBossBasicAttackAbility::StartAbility_Implementation(int32 AbilityIndex)
{
	if (bAbilityActive)
	{
		return;
	}
	AMVCharacterBase* Character = GetOwnerCharacter();
	USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
	if (!IsValid(Character) || !Character->GetWorld() || !Mesh
		|| !Mesh->DoesSocketExist(StartSocket) || !Mesh->DoesSocketExist(EndSocket))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossBasicAttack] Invalid trace sockets: Owner=%s Start=%s End=%s"),
			*GetNameSafe(Character), *StartSocket.ToString(), *EndSocket.ToString());
		return;
	}
	Super::StartAbility_Implementation(AbilityIndex);
	if (!bAbilityActive)
	{
		return;
	}
	HitActors.Reset();
	TraceWorld = Character->GetWorld();
	TraceWorld->GetTimerManager().SetTimer(TraceTimer, this,
		&UMVTutorialBossBasicAttackAbility::TraceAttack, 1.0f / 60.0f, true);
	UE_LOG(LogTemp, Display, TEXT("[BossBasicAttack] Begin Frame=%llu Owner=%s AttackId=%d Index=%d Radius=%.1f"),
		GFrameCounter, *GetNameSafe(Character), GetAttackInstanceId(), AbilityIndex, TraceRadius);
	TraceAttack();
}

void UMVTutorialBossBasicAttackAbility::ClearTraceTimer()
{
	if (UWorld* World = TraceWorld.Get())
	{
		World->GetTimerManager().ClearTimer(TraceTimer);
	}
	TraceWorld.Reset();
}

void UMVTutorialBossBasicAttackAbility::EndAbility_Implementation()
{
	ClearTraceTimer();
	if (bAbilityActive)
	{
		UE_LOG(LogTemp, Display, TEXT("[BossBasicAttack] End Frame=%llu Owner=%s AttackId=%d Hits=%d"),
			GFrameCounter, *GetNameSafe(GetOwnerCharacter()), GetAttackInstanceId(), HitActors.Num());
	}
	HitActors.Reset();
	Super::EndAbility_Implementation();
}

void UMVTutorialBossBasicAttackAbility::BeginDestroy()
{
	ClearTraceTimer();
	Super::BeginDestroy();
}

void UMVTutorialBossBasicAttackAbility::TraceAttack()
{
	AMVCharacterBase* Character = GetOwnerCharacter();
	UWorld* World = TraceWorld.Get();
	if (!bAbilityActive || !IsValid(Character) || !World || Character->IsActorBeingDestroyed()
		|| (Character->StatComponent && Character->StatComponent->IsDead()))
	{
		EndAbility_Implementation();
		return;
	}
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	const FVector Start = Mesh->GetSocketLocation(StartSocket);
	const FVector End = Mesh->GetSocketLocation(EndSocket);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TutorialBossBasicAttack), false, Character);
	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(TraceRadius), Params);
	bool bHit = false;
	for (const FHitResult& Hit : Hits)
	{
		AMVCharacterBase* Victim = Cast<AMVCharacterBase>(Hit.GetActor());
		if (!IsValid(Victim) || !Victim->IsPlayerControlled() || Victim->IsInvincible() || HitActors.Contains(Victim)
			|| (Victim->StatComponent && Victim->StatComponent->IsDead()))
		{
			continue;
		}
		UMVHitResolverSubsystem* Resolver = UMVHitResolverSubsystem::Get(World);
		if (!Resolver)
		{
			continue;
		}
		FMVHitResolveRequest Request;
		Request.Attacker = Character;
		Request.Victim = Victim;
		Request.AttackInstanceId = GetAttackInstanceId();
		Request.DamageMultiplier = AbilityData.DamageMultiplier;
		Request.GroggyDamageMultiplier = AbilityData.GroggyDamageMultiplier;
		Request.HitReactionType = EMVActionHitReactionType::Flinch;
		Request.HitLocation = Hit.ImpactPoint;
		Request.ImpactNormal = Hit.ImpactNormal;
		Request.HitLaunchData = HitLaunchData;
		FMVResolvedHitData Resolved;
		// 결과 브로드캐스트 중 재진입에도 동일 구간의 중복 타격 방지
		HitActors.Add(Victim);
		if (Resolver->ResolveAttackHit(Request, Resolved))
		{
			bHit = true;
			UE_LOG(LogTemp, Display, TEXT("[BossBasicAttack] Hit Frame=%llu Owner=%s Victim=%s AttackId=%d Damage=%.1f"),
				GFrameCounter, *GetNameSafe(Character), *GetNameSafe(Victim), GetAttackInstanceId(), Resolved.FinalDamage);
		}
		else
		{
			HitActors.Remove(Victim);
		}
	}
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		const FVector Segment = End - Start;
		DrawDebugCapsule(World, (Start + End) * 0.5f, Segment.Size() * 0.5f + TraceRadius,
			TraceRadius, FQuat::FindBetweenNormals(FVector::UpVector, Segment.GetSafeNormal(SMALL_NUMBER, FVector::UpVector)),
			bHit ? FColor::Green : FColor::Red, false, 0.05f);
	}
#endif
}
