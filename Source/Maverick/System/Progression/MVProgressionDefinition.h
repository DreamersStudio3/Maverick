#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Struct/MVProgressionTypes.h"
#include "MVProgressionDefinition.generated.h"

/**
 * 레벨 비용과 투자 항목별 누적 스탯 보너스를 정의하는 데이터 에셋.
 *
 * 기본 생성값으로 HP, Stamina, MP 테스트 항목을 제공한다.
 * 런타임 계산기는 이 정의를 읽기만 하며 투자량, 재화, 캐릭터 상태를 소유하지 않는다.
 */
UCLASS(BlueprintType)
class MAVERICK_API UMVProgressionDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UMVProgressionDefinition();

	const FMVProgressionEntryDefinition* FindEntry(FGameplayTag ProgressionId) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (ClampMin = "1"))
	int32 BaseLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (ClampMin = "1"))
	int32 MaxLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (ClampMin = "0"))
	int64 BaseLevelCost = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (ClampMin = "0"))
	int64 CostPerLevel = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression")
	TArray<FMVProgressionEntryDefinition> Entries;
};