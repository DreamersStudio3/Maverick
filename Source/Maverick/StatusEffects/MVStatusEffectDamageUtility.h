#pragma once

#include "CoreMinimal.h"

class AActor;
struct FMVResolvedHitData;

/**
 * 상태 효과 Behavior가 사용하는 공용 피해 실행 함수
 *
 * 최종 피해량을 직접 계산한 뒤 ResolveDirectDamage 경로로 전달
 * 일반 공격력·무기 공격력·방어력 계산은 적용하지 않음
 */
namespace MVStatusEffectDamage
{
	MAVERICK_API bool ApplyTargetMaxHealthDamage(
		const UObject* WorldContextObject,
		AActor* SourceActor,
		AActor* TargetActor,
		float DamageRatio,
		FMVResolvedHitData* OutHitData = nullptr);
}