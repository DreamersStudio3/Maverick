#pragma once

#include "CoreMinimal.h"
#include "MVWorldStateTypes.generated.h"

/**
 * WorldState 저장 데이터의 공통 레코드 타입.
 *
 * `UMVWorldStateSubsystem`과 `UMVWorldSaveGame`이 공유하는 순수 데이터 구조만 둔다.
 * 런타임 정책은 subsystem이 담당하고, 이 파일은 마지막 체크포인트, 1회성 필드 오브젝트,
 * 월드 플래그, 퀘스트 진행처럼 세이브 슬롯에 직렬화되어야 하는 값만 표현한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVCheckpointSaveData
{
	GENERATED_BODY()

	FMVCheckpointSaveData()
		: Transform(FTransform::Identity)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Checkpoint")
	bool bHasCheckpoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Checkpoint")
	FName CheckpointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Checkpoint")
	FName FieldId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Checkpoint")
	FName MapName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Checkpoint")
	FTransform Transform;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVFieldObjectSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Field")
	FName FieldId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Field")
	FName ObjectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Field")
	bool bConsumed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Field")
	FName StateId = NAME_None;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVWorldFlagSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|World")
	FName FlagId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|World")
	bool bValue = false;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVQuestSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Quest")
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Quest")
	FName StateId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Quest")
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Quest")
	TMap<FName, int32> ObjectiveValues;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVWorldSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	int32 SaveVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	FString SavedAtUtcIso8601;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	FMVCheckpointSaveData LastCheckpoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	TArray<FMVFieldObjectSaveData> FieldObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	TArray<FMVWorldFlagSaveData> WorldFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	TArray<FMVQuestSaveData> Quests;
};
