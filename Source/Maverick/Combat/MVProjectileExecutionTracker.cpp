#include "Combat/MVProjectileExecutionTracker.h"

#include "Character/MVCharacterBase.h"
#include "Combat/MVAbilityBase.h"
#include "Combat/Projectile/MVProjectileBase.h"
#include "Components/ActorComponent.h"
#include "UObject/StrongObjectPtr.h"

bool UMVProjectileExecutionTracker::Register(
	AMVProjectileBase* Projectile,
	const FMVProjectileExecutionRecord& Source)
{
	UMVAbilityBase* SourceAbility = Source.SourceAbility.Get();
	UActorComponent* OwnerComponent = GetTypedOuter<UActorComponent>();
	if (!IsValid(Projectile) || !IsValid(SourceAbility) || !IsValid(OwnerComponent))
	{
		return false;
	}

	const int32 AttackInstanceId = Projectile->GetAttackInstanceId();
	const TObjectKey<AMVProjectileBase> ProjectileKey(Projectile);
	FMVProjectileExecutionRecord* Record = Records.Find(AttackInstanceId);
	if (Record)
	{
		if (Record->SourceAbility.Get() != SourceAbility ||
			!IsValid(Record->ExecutionAbility.Get()) ||
			Record->SourceLifeId != Source.SourceLifeId ||
			Record->bActionEnded ||
			Record->ActiveProjectiles.Contains(ProjectileKey))
		{
			return false;
		}
	}
	else
	{
		// 원본 Ability는 다음 공격에서 다시 준비되므로 실행 값을 복사한다.
		UMVAbilityBase* ExecutionAbility = NewObject<UMVAbilityBase>(OwnerComponent);
		if (!IsValid(ExecutionAbility))
		{
			return false;
		}

		ExecutionAbility->SetOwner(OwnerComponent);
		ExecutionAbility->AbilityData = SourceAbility->AbilityData;
		ExecutionAbility->SetAttackInstanceId(AttackInstanceId);
		ExecutionAbility->SetExecutionSource(SourceAbility->GetSourceSkillIndex(), Source.SourceWeaponItemTag);
		ExecutionAbility->ConsumedMPThisExecution = SourceAbility->GetConsumedMPThisExecution();
		ExecutionAbility->bAbilityCostConsumed = SourceAbility->bAbilityCostConsumed;
		ExecutionAbility->HitLaunchData = SourceAbility->GetHitLaunchData();

		Record = &Records.Add(AttackInstanceId, Source);
		Record->ExecutionAbility = ExecutionAbility;
	}

	Record->ActiveProjectiles.Add(ProjectileKey);
	Projectile->OnProjectileHit.AddUniqueDynamic(this, &UMVProjectileExecutionTracker::HandleProjectileHit);
	Projectile->OnProjectileFinished.AddUniqueDynamic(this, &UMVProjectileExecutionTracker::HandleProjectileFinished);
	return true;
}

const FMVProjectileExecutionRecord* UMVProjectileExecutionTracker::Find(int32 AttackInstanceId) const
{
	return Records.Find(AttackInstanceId);
}

bool UMVProjectileExecutionTracker::TryConsumeRefund(int32 AttackInstanceId)
{
	FMVProjectileExecutionRecord* Record = Records.Find(AttackInstanceId);
	if (!Record || Record->bRefundProcessed)
	{
		return false;
	}

	Record->bRefundProcessed = true;
	return true;
}

bool UMVProjectileExecutionTracker::QueueMiss(int32 AttackInstanceId, const UMVAbilityBase* SourceAbility)
{
	FMVProjectileExecutionRecord* Record = Records.Find(AttackInstanceId);
	if (!Record || Record->SourceAbility.Get() != SourceAbility)
	{
		return false;
	}

	Record->bMissPending = true;
	return true;
}

void UMVProjectileExecutionTracker::MarkHit(int32 AttackInstanceId)
{
	if (FMVProjectileExecutionRecord* Record = Records.Find(AttackInstanceId))
	{
		Record->bHadHit = true;
	}
}

void UMVProjectileExecutionTracker::CloseAction(int32 AttackInstanceId)
{
	if (FMVProjectileExecutionRecord* Record = Records.Find(AttackInstanceId))
	{
		Record->bActionEnded = true;
		Finalize(AttackInstanceId);
	}
}

void UMVProjectileExecutionTracker::Reset()
{
	OnMissed.Unbind();
	for (const TPair<int32, FMVProjectileExecutionRecord>& Pair : Records)
	{
		for (const TObjectKey<AMVProjectileBase>& ProjectileKey : Pair.Value.ActiveProjectiles)
		{
			if (AMVProjectileBase* Projectile = ProjectileKey.ResolveObjectPtr())
			{
				Projectile->OnProjectileHit.RemoveDynamic(this, &UMVProjectileExecutionTracker::HandleProjectileHit);
				Projectile->OnProjectileFinished.RemoveDynamic(this, &UMVProjectileExecutionTracker::HandleProjectileFinished);
			}
		}
	}
	Records.Reset();
}

void UMVProjectileExecutionTracker::HandleProjectileHit(
	AMVProjectileBase* Projectile,
	const FMVResolvedHitData& HitData)
{
	const UActorComponent* OwnerComponent = GetTypedOuter<UActorComponent>();
	if (!IsValid(Projectile) || HitData.AttackInstanceId != Projectile->GetAttackInstanceId() ||
		!IsValid(OwnerComponent) || HitData.Attacker.Get() != OwnerComponent->GetOwner())
	{
		return;
	}

	FMVProjectileExecutionRecord* Record = Records.Find(HitData.AttackInstanceId);
	if (Record && Record->ActiveProjectiles.Contains(TObjectKey<AMVProjectileBase>(Projectile)))
	{
		Record->bHadActivatedProjectile |= Projectile->WasEverActivated();
		Record->bHadHit = true;
	}
}

void UMVProjectileExecutionTracker::HandleProjectileFinished(
	AMVProjectileBase* Projectile,
	int32 AttackInstanceId)
{
	FMVProjectileExecutionRecord* Record = Records.Find(AttackInstanceId);
	if (!Record || !Projectile ||
		Record->ActiveProjectiles.Remove(TObjectKey<AMVProjectileBase>(Projectile)) == 0)
	{
		return;
	}

	Record->bHadActivatedProjectile |= Projectile->WasEverActivated();
	Finalize(AttackInstanceId);
}

void UMVProjectileExecutionTracker::Finalize(int32 AttackInstanceId)
{
	FMVProjectileExecutionRecord* Record = Records.Find(AttackInstanceId);
	if (!Record || !Record->bActionEnded || !Record->ActiveProjectiles.IsEmpty() || Record->bNotifyingMiss)
	{
		return;
	}

	if (Record->bMissPending && Record->bHadActivatedProjectile && !Record->bHadHit)
	{
		Record->bNotifyingMiss = true;
		const FMVProjectileExecutionRecord Missed = *Record;
		const TStrongObjectPtr<UMVAbilityBase> KeepAbilityAlive(Missed.ExecutionAbility.Get());
		OnMissed.ExecuteIfBound(Missed);
	}

	Records.Remove(AttackInstanceId);
}
