#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "System/MVWorldStateTypes.h"
#include "MVWorldStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMVOnWorldStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnWorldStateSlotEvent, FString, SlotName);

/**
 * 저장되어야 하는 전체 게임 상태를 소유하는 GameInstance 서브시스템.
 *
 * 마지막 체크포인트, 1회성 필드 오브젝트 소비 상태, 영구 월드 플래그, 퀘스트 진행 상태를
 * `FMVWorldSaveData`로 모아 `UMVWorldSaveGame` 슬롯에 저장/로드한다. 퀘스트나 부활 흐름 같은
 * 도메인 시스템은 SaveGame을 직접 수정하지 않고 이 서브시스템에 변경을 요청한다.
 *
 * 라이프사이클:
 *   1) GameInstance 생성 시 빈 저장 데이터를 준비한다.
 *   2) 저장 지점, 퀘스트, 1회성 스폰 상태 변경 요청을 받아 in-memory save data를 갱신한다.
 *   3) Save/Load 요청 시 Unreal SaveGame API로 슬롯을 쓰거나 읽고 변경 이벤트를 발행한다.
 */
UCLASS()
class MAVERICK_API UMVWorldStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UMVWorldStateSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	bool SaveToSlot(const FString& SlotName, int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	bool SaveToDefaultSlot();

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	bool LoadFromSlot(const FString& SlotName, int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	bool LoadDefaultSlot();

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	bool LoadOrCreateDefaultSlot();

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	bool DeleteSaveSlot(const FString& SlotName, int32 UserIndex);

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Save")
	bool DoesSaveSlotExist(const FString& SlotName, int32 UserIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	void ResetSaveData();

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Save")
	void ApplySaveData(const FMVWorldSaveData& InSaveData);

	const FMVWorldSaveData& GetSaveData() const { return CurrentSaveData; }

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Save")
	FMVWorldSaveData GetSaveDataCopy() const { return CurrentSaveData; }

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Save")
	bool IsSaveDataDirty() const { return bSaveDataDirty; }

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Checkpoint")
	bool SetLastCheckpoint(FName CheckpointId, FName FieldId, const FTransform& Transform, FName MapName);

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Checkpoint")
	bool TryGetLastCheckpoint(FMVCheckpointSaveData& OutCheckpoint) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Checkpoint")
	bool HasLastCheckpoint() const { return CurrentSaveData.LastCheckpoint.bHasCheckpoint; }

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Field")
	bool SetFieldObjectState(FName FieldId, FName ObjectId, bool bConsumed, FName StateId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Field")
	bool MarkOneTimeSpawnConsumed(FName FieldId, FName ObjectId);

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Field")
	bool IsOneTimeSpawnConsumed(FName FieldId, FName ObjectId) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Flag")
	bool SetWorldFlag(FName FlagId, bool bValue);

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Flag")
	bool GetWorldFlag(FName FlagId, bool bDefaultValue) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Quest")
	bool SetQuestState(FName QuestId, FName StateId);

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Quest")
	FName GetQuestState(FName QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Quest")
	bool SetQuestObjectiveValue(FName QuestId, FName ObjectiveId, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Quest")
	int32 GetQuestObjectiveValue(FName QuestId, FName ObjectiveId, int32 DefaultValue) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|WorldState|Quest")
	bool CompleteQuest(FName QuestId);

	UFUNCTION(BlueprintPure, Category = "Maverick|WorldState|Quest")
	bool IsQuestCompleted(FName QuestId) const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|WorldState|Event")
	FMVOnWorldStateChanged OnWorldStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|WorldState|Event")
	FMVOnWorldStateSlotEvent OnWorldStateSaved;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|WorldState|Event")
	FMVOnWorldStateSlotEvent OnWorldStateLoaded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|WorldState|Save")
	FString DefaultSaveSlotName = TEXT("MaverickDefault");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|WorldState|Save")
	int32 DefaultUserIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|WorldState|Save")
	FString ActiveSaveSlotName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|WorldState|Save")
	int32 ActiveUserIndex = 0;

private:
	void MarkSaveDataDirty();
	FString ResolveSlotName(const FString& SlotName) const;
	FName ResolveCurrentMapName() const;
	FMVFieldObjectSaveData* FindFieldObjectRecord(FName FieldId, FName ObjectId);
	const FMVFieldObjectSaveData* FindFieldObjectRecord(FName FieldId, FName ObjectId) const;
	FMVWorldFlagSaveData* FindWorldFlagRecord(FName FlagId);
	const FMVWorldFlagSaveData* FindWorldFlagRecord(FName FlagId) const;
	FMVQuestSaveData* FindQuestRecord(FName QuestId);
	const FMVQuestSaveData* FindQuestRecord(FName QuestId) const;
	FMVQuestSaveData& FindOrAddQuestRecord(FName QuestId);

	UPROPERTY(Transient)
	FMVWorldSaveData CurrentSaveData;

	bool bSaveDataDirty = false;
};
