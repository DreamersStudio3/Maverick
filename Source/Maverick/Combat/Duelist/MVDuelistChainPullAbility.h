#pragma once

#include "CoreMinimal.h"
#include "Combat/MVAbilityBase.h"
#include "MVDuelistChainPullAbility.generated.h"

class AMVDuelistChainProjectile;

/**
 * Duelist 돌진 사슬 공격의 대상·발사 1회·투사체 수명 소유
 * CombatComponent 준비 → Ability NotifyState 시작 → 발사 Notify → 종료·취소 시 투사체 정리
 * 돌진 이동은 몽타주 루트 모션, 비행·끌어오기는 전용 투사체 Actor 책임
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVDuelistChainPullAbility : public UMVAbilityBase
{
	GENERATED_BODY()

public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	virtual void EndAbility_Implementation() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Duelist|Chain")
	void FireChain();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chain")
	FName LaunchSocket = TEXT("BN_Weapon_RSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chain", meta = (ClampMin = "1.0", Units = "cm/s"))
	float ProjectileSpeed = 2400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chain", meta = (ClampMin = "1.0", Units = "cm"))
	float MaxRange = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chain", meta = (ClampMin = "0.1", Units = "s"))
	float PullDuration = 0.65f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chain", meta = (ClampMin = "1.0", Units = "cm"))
	float StopDistance = 180.0f;

private:
	TWeakObjectPtr<AMVCharacterBase> Target;
	TWeakObjectPtr<AMVDuelistChainProjectile> Projectile;
	bool bFired = false;
	void ClearProjectile();
};
