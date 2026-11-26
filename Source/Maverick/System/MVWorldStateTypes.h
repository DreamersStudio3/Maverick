#pragma once

#include "CoreMinimal.h"
#include "Enum/MVEquipmentEnums.h"
#include "Enum/MVSkillSlot.h"
#include "Struct/MVProgressionTypes.h"
#include "Tables/MVEnemyDropTableTypes.h"
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

/** 완료한 보스 등장 연출의 맵별 식별자·최종 월드 변환, 런타임 액터 참조 제외 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVBossIntroSaveData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|BossIntro")
	FName IntroId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|BossIntro")
	FName MapName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|BossIntro")
	FTransform BossTransform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVRestPointDiscovery
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Save")
	FName MapName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Save")
	FName FieldId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Save")
	FName RestPointId;
};

class UDataTable;

/** 데이터 테이블 자산 경로와 행 이름으로 저장하며 런타임 행 포인터는 보관하지 않는다. */
USTRUCT()
struct MAVERICK_API FMVSavedWeaponRow
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UDataTable> Table;

	UPROPERTY()
	FName RowName;
};

USTRUCT()
struct MAVERICK_API FMVSavedWeaponSkillStyle
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FMVSavedWeaponRow> UnlockedRows;

	UPROPERTY()
	TMap<EMVSkillSlot, FMVSavedWeaponRow> EquippedRows;
};

USTRUCT()
struct MAVERICK_API FMVWeaponProgressSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasRecord = false;

	// 4버전 저장에서만 읽는 이전 무기 소유 자료
	UPROPERTY()
	TMap<FGameplayTag, FMVSavedWeaponRow> OwnedWeapons;

	UPROPERTY()
	TArray<FGameplayTag> QuickSlots;

	UPROPERTY()
	FGameplayTag EquippedWeapon;

	UPROPERTY()
	int32 ActiveSlot = INDEX_NONE;

	UPROPERTY()
	TMap<EMVEquippedStyle, FMVSavedWeaponSkillStyle> SkillStyles;

	UPROPERTY()
	TMap<FGameplayTag, FMVSavedWeaponRow> SelectedPassives;

	UPROPERTY()
	int32 SkillPoints = 0;
};

/** 기록 유무를 따로 저장해 처음 시작하는 가방과 저장된 빈 가방을 구분한다. */
USTRUCT()
struct MAVERICK_API FMVInventorySaveData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasRecord = false;

	UPROPERTY()
	TMap<FGameplayTag, int32> ItemCounts;

	UPROPERTY()
	TSet<FGameplayTag> DiscoveredItems;
};

/** 한 번의 처치에서 아직 받지 못한 품목과 수량. 순서는 보상 행을 따른다. */
USTRUCT()
struct MAVERICK_API FMVPendingEnemyReward
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid DefeatId;

	// 저장을 다시 불러와도 처치 직후의 표시 간격을 이어간다. 이전 저장의 기본값은 이미 지급 가능하다.
	UPROPERTY()
	FDateTime AvailableAtUtc = FDateTime::MinValue();

	UPROPERTY()
	TArray<FMVGuaranteedEnemyDrop> Items;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Save")
	TArray<FMVRestPointDiscovery> DiscoveredRestPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Save")
	FMVCheckpointSaveData LastRestPoint;

	UPROPERTY()
	FMVWeaponProgressSaveData WeaponProgress;

	UPROPERTY()
	FMVInventorySaveData Inventory;

	UPROPERTY()
	TArray<FMVPendingEnemyReward> PendingEnemyRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	TArray<FMVFieldObjectSaveData> FieldObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	TArray<FMVWorldFlagSaveData> WorldFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save")
	TArray<FMVQuestSaveData> Quests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|BossIntro")
	TArray<FMVBossIntroSaveData> BossIntros;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Save|Progression")
	FMVPlayerProgressionSaveData PlayerProgression;
};
