#pragma once

#include "CoreMinimal.h"
#include "Tables/MVTableTypes.h"
#include "UObject/SoftObjectPath.h"
#include "MVActionRowTableTypes.generated.h"

/**
 * Combat/HitReaction 같은 도메인 컴포넌트가 선택한 상황별 액션의 공용 실행 row.
 *
 * 도메인 컴포넌트는 ChooserTable을 통해 상황별 DataTable row handle을 고르고,
 * ActionComponent는 그 row를 읽어 몽타주 재생과 실행 이벤트 전파만 담당한다.
 * 자원 소모, 대미지, 런치처럼 도메인별로 다른 데이터는 이 구조체를 상속한 전용 row에 둔다.
 * ActionComponent는 이 row를 선택하지 않고 실행만 담당한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVActionRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|ActionRow|Animation")
	FSoftObjectPath Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|ActionRow|Animation")
	FName DefaultStartSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|ActionRow|Animation", meta = (ClampMin = "0.0"))
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|ActionRow")
	bool bLocksMovement = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|ActionRow")
	bool bCanBeInterrupted = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|ActionRow")
	bool bEnabled = true;
};
