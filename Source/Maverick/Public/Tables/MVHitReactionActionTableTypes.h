#pragma once

#include "CoreMinimal.h"
#include "Tables/MVActionRowTableTypes.h"
#include "MVHitReactionActionTableTypes.generated.h"

/**
 * HitReaction 전용 상황별 액션 row.
 *
 * 피격 리액션은 자원 소모 정책을 갖지 않고, 공용 액션 실행 필드에 피격 이동/런치 보조 데이터만 더한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitReactionActionRow : public FMVActionRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|HitReaction|Launch")
	bool bUseLaunch = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|HitReaction|Launch", meta = (ClampMin = "0.0", Units = "cm"))
	float LaunchDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|HitReaction|Launch", meta = (ClampMin = "0.0", Units = "s"))
	float LaunchDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|HitReaction|Launch", meta = (Units = "cm/s"))
	float LaunchVerticalSpeed = 0.0f;
};
