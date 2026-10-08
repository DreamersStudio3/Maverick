#pragma once

#include "Combat/MVAbilityBase.h"
#include "MVBoxDamageAbility.generated.h"

/**
 * 액터 기준 박스 안의 플레이어에게 활성 구간당 한 번의 HP 피해 적용
 * 크기·로컬 위치·디버그 표시를 Ability Blueprint 기본값에서 조절, 실제 피해 계산은 HitResolver에 위임
 * CombatComponent 준비 → MV Activate Ability 시작·검사 → 종료·취소·소멸 시 타이머 정리
 * 박스는 액터 위치·회전을 추적, 무적·사망·자기 자신 제외, 피격 동작·밀림·그로기 요청 없음
 */
UCLASS(Blueprintable)
class MAVERICK_API UMVBoxDamageAbility : public UMVAbilityBase
{
	GENERATED_BODY()

public:
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;
	virtual void EndAbility_Implementation() override;
	virtual void BeginDestroy() override;

	// X: 전후, Y: 좌우, Z: 높이의 전체 길이
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attack|Box", meta = (ClampMin = "1.0", Units = "cm"))
	FVector BoxSize = FVector(200.0f, 200.0f, 200.0f);

	// 액터의 전방·우측·위 방향 기준 박스 중심 위치, 메시 스케일과 무관한 cm 단위
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attack|Box", meta = (Units = "cm"))
	FVector BoxOffset = FVector(150.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attack|Box|Debug")
	bool bDrawDebug = true;

private:
	void CheckBoxDamage();
	void ClearBoxTimer();
	FTimerHandle BoxTimer;
	TWeakObjectPtr<UWorld> BoxWorld;
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
