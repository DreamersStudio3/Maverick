#pragma once

#include "CoreMinimal.h"
#include "Combat/MVAbilityBase.h"
#include "MVWeaponCounterAbility.generated.h"

/** 방어 Notify 구간에 1회 차단을 등록한다. 차단을 소비하면 방어를 닫고 같은 실행에 반격 판정을 귀속한다. */
UCLASS(Blueprintable)
class MAVERICK_API UMVWeaponCounterAbility : public UMVAbilityBase
{
	GENERATED_BODY()

public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	virtual void EndAbility_Implementation() override;
	virtual void BeginDestroy() override;

protected:
	// X 전방 깊이, Y 좌우 폭, Z 높이 전체 치수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Weapon Skill", meta = (ClampMin = "1.0", Units = "cm"))
	FVector CounterBoxSize = FVector(100.0, 500.0, 300.0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Weapon Skill", meta = (ClampMin = "0.01", Units = "s"))
	float CounterDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Weapon Skill")
	bool bDrawCounterBox = false;

private:
	UFUNCTION()
	void HandleBlockedHit(const FMVResolvedHitData& HitData);

	void CheckCounterBox();
	void FinishCounter();
	void ClearCounterTimers();

	TWeakObjectPtr<AMVCharacterBase> Defender;
	TWeakObjectPtr<UWorld> CounterWorld;
	TSet<TWeakObjectPtr<AMVCharacterBase>> HitCharacters;
	FTimerHandle CounterCheckTimer;
	FTimerHandle CounterEndTimer;
	FVector BoxCenter = FVector::ZeroVector;
	FQuat BoxRotation = FQuat::Identity;
	int32 DefenseRegistrationId = INDEX_NONE;
	int32 LastDefenseAttackId = INDEX_NONE;
	int32 CounterAttackId = INDEX_NONE;
	int32 CounterActionId = INDEX_NONE;
	double CounterEndTime = 0.0;
};
