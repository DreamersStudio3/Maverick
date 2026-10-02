#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UMVStatComponent;

/**
 * 성장 스탯 ID와 실제 캐릭터 수치·표시 정보를 연결하는 등록부
 *
 * 기본값·실효값 조회와 성장 보너스 적용 경로 제공
 * 표시 이름과 수치의 유리한 변경 방향도 이 등록부에서 관리
 * 저장 데이터와 성장 보너스의 소유권은 외부 유지
 */
class MAVERICK_API FMVProgressionStatBinding
{
public:
	static TArray<FGameplayTag> GetSupportedStatIds();
	static bool IsSupportedStat(const FGameplayTag& StatId);
	static bool TryGetDisplayInfo(const FGameplayTag& StatId, FText& OutDisplayName, bool& bOutLowerIsBetter);
	static bool TryGetBaseValue(const UMVStatComponent& StatComponent, const FGameplayTag& StatId, float& OutValue);
	static bool TryGetEffectiveValue(const UMVStatComponent& StatComponent, const FGameplayTag& StatId, float& OutValue);
	static bool TryApplyBonus(UMVStatComponent& StatComponent, const FGameplayTag& StatId, float Bonus);
};