#pragma once

#include "Combat/MVAbilityBase.h"
#include "MVBoxDamageAbility.generated.h"

/**
 * 액터 기준 박스 안의 플레이어에게 활성 구간당 한 번의 피해 적용
 * BP_BoxCollision과 에어본 설정 에셋의 공통 판정, 실제 피해·피격 동작은 HitResolver에 위임
 * CombatComponent 준비 → MV Activate Ability 시작·검사 → 종료·취소·소멸 시 타이머 정리
 * 크기·위치·피격 유형은 Blueprint 기본값, None 선택 시 밀림·그로기 없이 HP 피해만 요청
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

	// None: HP 피해만 적용, 그 외: 행의 그로기 배율과 기존 HitLaunchData 사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attack|Box")
	EMVActionHitReactionType HitReactionType = EMVActionHitReactionType::None;

private:
	void CheckBoxDamage();
	void ClearBoxTimer();
	FTimerHandle BoxTimer;
	TWeakObjectPtr<UWorld> BoxWorld;
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
