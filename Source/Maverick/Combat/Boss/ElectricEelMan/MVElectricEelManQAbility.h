#pragma once

#include "Combat/MVBoxDamageAbility.h"
#include "MVElectricEelManQAbility.generated.h"

/**
 * 전기뱀장어맨 Q의 양손 꼬리 내려치기를 좌우 두 개의 긴 박스로 판정
 * BoxOffset을 두 꼬리의 중간 중심으로 사용, BoxSize는 꼬리 하나의 전체 판정 크기
 * MV Activate Ability 구간 동안 부모의 검사·비용·중복 적중 방지·종료 정리 재사용
 * 같은 플레이어는 두 박스 전체에서 활성 구간당 한 번 피격, 꼬리 변형·모션은 몽타주 책임
 */
UCLASS(Blueprintable, meta = (DisplayName = "Electric Eel Man Q - Tail Slam"))
class MAVERICK_API UMVElectricEelManQAbility : public UMVBoxDamageAbility
{
	GENERATED_BODY()

public:
	UMVElectricEelManQAbility();
	virtual void StartAbility_Implementation(int32 AbilityIndex) override;

	// 좌우 꼬리 판정 중심 사이의 전체 간격, 액터 로컬 Y축 기준
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attack|Box", meta = (ClampMin = "0.0", Units = "cm"))
	float TailSeparation = 180.0f;

protected:
	virtual void GetBoxOffsets(TArray<FVector>& OutOffsets) const override;
};
