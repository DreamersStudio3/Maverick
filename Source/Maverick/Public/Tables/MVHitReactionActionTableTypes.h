#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Tables/MVActionRowTableTypes.h"
#include "MVHitReactionActionTableTypes.generated.h"

/**
 * HitReaction 전용 상황별 액션 row.
 *
 * 피격 리액션은 자원 소모 정책을 갖지 않고, 공용 액션 실행 필드에 Launch 사용 여부만 더한다.
 * 실제 Launch 세기는 공격 Ability가 FMVResolvedHitData로 전달한 값을 사용한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitReactionActionRow : public FMVActionRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|HitReaction|Launch")
	bool bUseLaunch = false;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVHitReactionActionRowHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Table|HitReaction")
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Table|HitReaction")
	FName StartSection = NAME_None;

	bool IsValid() const
	{
		return ActionRow.DataTable && !ActionRow.RowName.IsNone();
	}

	void Reset()
	{
		ActionRow.DataTable = nullptr;
		ActionRow.RowName = NAME_None;
		StartSection = NAME_None;
	}
};
