#pragma once

#include "Combat/MVAbilityBase.h"
#include "MVTutorialBossBasicAttackAbility.generated.h"

/**
 * TutorialBoss BasicAttack1의 무기 소켓 타격 검사와 범위 표시 책임
 * CombatComponent 생성·준비 후 기존 MV Activate Ability 구간에서만 검사 활성화
 * Notify 구간별 대상 1회 타격, 종료·액션 취소 시 타이머와 대상 목록 정리
 * 피해 계산·체력 변경은 기존 HitResolver와 피격 컴포넌트 경로에 위임
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVTutorialBossBasicAttackAbility : public UMVAbilityBase
{
	GENERATED_BODY()

public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	virtual void EndAbility_Implementation() override;
	virtual void BeginDestroy() override;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
	FName StartSocket = TEXT("BN_Weapon_RSocket");

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
	FName EndSocket = TEXT("Trail_Socket_R");

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace", meta = (ClampMin = "1.0", Units = "cm"))
	float TraceRadius = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
	bool bDrawDebug = true;

private:
	void TraceAttack();
	void ClearTraceTimer();
	FTimerHandle TraceTimer;
	TWeakObjectPtr<UWorld> TraceWorld;
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
