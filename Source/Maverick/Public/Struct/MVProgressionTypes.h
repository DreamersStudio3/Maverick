#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "GameplayTagContainer.h"
#include "MVProgressionTypes.generated.h"

UENUM(BlueprintType)
enum class EMVProgressionEvaluationResult : uint8
{
	Success,
	MissingDefinition,
	InvalidDefinition,
	InvalidSaveData,
	UnknownProgression,
	NegativeAllocation,
	NoAllocation,
	RankLimitExceeded,
	LevelLimitExceeded,
	InsufficientCurrency,
	RevisionMismatch,
	StateWriteFailed,
	CostOverflow,
	InvalidCurve
};

/**
 * 하나의 성장 항목이 실제 스탯에 주는 누적 효과.
 *
 * BonusByRank는 해당 투자 rank에서의 전체 보너스를 반환한다.
 * 계산 시 rank 0 값을 기준점으로 빼므로 곡선 시작값이 0이 아니어도 처리 가능하다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVProgressionStatEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (Categories = "Stat"))
	FGameplayTag StatId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression")
	FRuntimeFloatCurve BonusByRank;
};

/**
 * UI에 표시되는 투자 항목 하나의 정의.
 *
 * ProgressionId와 실제 StatId를 분리해 하나의 투자 항목이
 * 여러 실제 스탯에 영향을 줄 수 있도록 구성한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVProgressionEntryDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (Categories = "Progression.Attribute"))
	FGameplayTag ProgressionId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression", meta = (ClampMin = "0"))
	int32 MaxRank = 99;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression")
	int32 SortOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Progression")
	TArray<FMVProgressionStatEffect> Effects;
};

/**
 * 저장 슬롯에 기록되는 플레이어 성장 원본.
 *
 * 투자량과 재화만 영구 보관하며 계산된 레벨과 실효 스탯은 저장하지 않는다.
 * 레벨은 BaseLevel과 InvestedRanks 합계로 계산한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVPlayerProgressionSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Progression")
	int32 DataVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Progression", meta = (ClampMin = "1"))
	int32 BaseLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Progression")
	TMap<FGameplayTag, int32> InvestedRanks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Progression", meta = (ClampMin = "0"))
	int64 Currency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Progression", meta = (ClampMin = "0"))
	int32 Revision = 0;
};

/**
 * 임시 배분을 평가한 읽기 전용 결과.
 *
 * 계산기는 이 구조체만 반환하며 저장 데이터와 캐릭터 스탯을 직접 변경하지 않는다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVProgressionEvaluation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	EMVProgressionEvaluationResult Result = EMVProgressionEvaluationResult::InvalidDefinition;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	FString Diagnostic;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	int32 SourceRevision = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	int32 ResultRevision = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	int32 PreviewLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	int64 TotalCost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	int64 CurrentCurrency = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	int64 RemainingCurrency = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	TMap<FGameplayTag, int32> PreviewRanks;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	TMap<FGameplayTag, float> CurrentStatBonuses;

	UPROPERTY(BlueprintReadOnly, Category = "Maverick|Progression")
	TMap<FGameplayTag, float> PreviewStatBonuses;

	bool IsSuccess() const
	{
		return Result == EMVProgressionEvaluationResult::Success;
	}
};