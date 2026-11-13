#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Enum/MVSkillSlot.h"
#include "GameplayTagContainer.h"
#include "UObject/ObjectKey.h"
#include "MVProjectileExecutionTracker.generated.h"

class AMVProjectileBase;
class UMVAbilityBase;
struct FMVResolvedHitData;

/** 첫 탄 발사 시 고정한 공격 출처와 같은 공격에서 발사한 탄의 진행 상태. */
USTRUCT()
struct FMVProjectileExecutionRecord
{
	GENERATED_BODY()

	TWeakObjectPtr<UMVAbilityBase> SourceAbility;

	UPROPERTY(Transient)
	TObjectPtr<UMVAbilityBase> ExecutionAbility = nullptr;

	UPROPERTY(Transient)
	FDataTableRowHandle SourceRowHandle;

	EMVSkillSlot SourceSkillSlot = EMVSkillSlot::NONE;
	uint64 SourceSlotRevision = 0;
	uint64 SourceSkillUseId = 0;
	uint64 SourceLifeId = 0;
	FGameplayTag SourceWeaponItemTag;
	uint64 PassiveActivationId = 0;
	int32 SourceChainStageIndex = INDEX_NONE;
	bool bWasBasicAttack = false;
	bool bWasRSkill = false;

	bool HasHit() const { return bHadHit; }

private:
	friend class UMVProjectileExecutionTracker;

	TSet<TObjectKey<AMVProjectileBase>> ActiveProjectiles;
	bool bHadActivatedProjectile = false;
	bool bHadHit = false;
	bool bMissPending = false;
	bool bNotifyingMiss = false;
	bool bActionEnded = false;
	bool bRefundProcessed = false;
};

DECLARE_DELEGATE_OneParam(FMVOnTrackedProjectileMissed, const FMVProjectileExecutionRecord&);

/**
 * Combat이 소유하는 투사체 공격 추적 객체.
 *
 * 등록 시 실행 값을 보존하고, 공격 동작과 모든 탄이 끝나면 기록을 제거한다.
 * 빗나감 알림 중에는 패시브가 출처를 조회할 수 있도록 기록을 유지한다.
 * Combat 종료 시 탄 이벤트 구독을 해제한다.
 */
UCLASS()
class UMVProjectileExecutionTracker : public UObject
{
	GENERATED_BODY()

public:
	bool Register(AMVProjectileBase* Projectile, const FMVProjectileExecutionRecord& Source);
	const FMVProjectileExecutionRecord* Find(int32 AttackInstanceId) const;
	bool IsEmpty() const { return Records.IsEmpty(); }
	bool TryConsumeRefund(int32 AttackInstanceId);
	bool QueueMiss(int32 AttackInstanceId, const UMVAbilityBase* SourceAbility);
	void MarkHit(int32 AttackInstanceId);
	void CloseAction(int32 AttackInstanceId);
	void Reset();

	FMVOnTrackedProjectileMissed OnMissed;

private:
	void Finalize(int32 AttackInstanceId);

	UFUNCTION()
	void HandleProjectileHit(AMVProjectileBase* Projectile, const FMVResolvedHitData& HitData);

	UFUNCTION()
	void HandleProjectileFinished(AMVProjectileBase* Projectile, int32 AttackInstanceId);

	UPROPERTY(Transient)
	TMap<int32, FMVProjectileExecutionRecord> Records;
};
