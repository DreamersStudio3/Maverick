#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Struct/MVProgressionTypes.h"
#include "MVProgressionCalculatorLibrary.generated.h"

class UMVProgressionDefinition;

/**
 * 성장 정의와 저장 스냅샷으로 레벨업 미리보기를 계산하는 무상태 계산기.
 *
 * 미리보기와 확정 전 재검증에서 동일한 계산 경로를 제공한다.
 * 캐릭터 스탯, 저장 데이터, 재화를 직접 변경하지 않는다.
 */
UCLASS()
class MAVERICK_API UMVProgressionCalculatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Maverick|Progression")
	static FMVProgressionEvaluation EvaluateAllocation(
		const UMVProgressionDefinition* Definition,
		const FMVPlayerProgressionSaveData& SaveData,
		const TMap<FGameplayTag, int32>& PendingRanks);

	static bool CalculateTotalCost(
		const UMVProgressionDefinition& Definition,
		int32 StartingLevel,
		int32 LevelCount,
		int64& OutTotalCost);
};