#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StatusEffects/MVStatusEffectDefinition.h"
#include "MVWeaponPassiveSet.generated.h"

/**
 * 한 무기에 기본으로 달리는 패시브 정의들을 보관한다.
 * 장착 처리에서 이 자산을 읽어 각 패시브를 활성화한다.
 * 최종적으로 무기마다 세 정의를 설정한다.
 */
UCLASS(BlueprintType)
class MAVERICK_API UMVWeaponPassiveSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maverick|Weapon|Passive")
	TArray<TObjectPtr<UMVStatusEffectDefinition>> PassiveDefinitions;
};