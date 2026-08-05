#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVQuestSubsystem.generated.h"

class UMVWorldStateSubsystem;

/**
 * 퀘스트 규칙을 처리하고 저장 변경은 WorldState에 위임하는 GameInstance 서브시스템.
 *
 * 퀘스트 시작/진행/완료 같은 도메인 API를 제공하되, 저장 데이터의 실제 소유권은
 * `UMVWorldStateSubsystem`에 둔다. 이 구조는 퀘스트가 월드 플래그나 1회성 스폰 같은
 * 다른 영구 상태와 같은 저장 슬롯을 공유하더라도 SaveGame 포맷을 직접 알지 않게 한다.
 *
 * 라이프사이클:
 *   1) GameInstance 생성 시 WorldStateSubsystem 의존성을 초기화한다.
 *   2) 퀘스트 상태 변경 요청을 받으면 WorldStateSubsystem의 quest API를 호출한다.
 *   3) 필요할 때 기본 슬롯 저장을 요청해 변경 사항을 디스크에 반영한다.
 */
UCLASS()
class MAVERICK_API UMVQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UMVQuestSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Quest")
	bool SetQuestState(FName QuestId, FName StateId, bool bSaveImmediately = false);

	UFUNCTION(BlueprintPure, Category = "Maverick|Quest")
	FName GetQuestState(FName QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Quest")
	bool SetQuestObjectiveValue(FName QuestId, FName ObjectiveId, int32 Value, bool bSaveImmediately = false);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Quest")
	bool AddQuestObjectiveValue(FName QuestId, FName ObjectiveId, int32 Delta, bool bSaveImmediately = false);

	UFUNCTION(BlueprintPure, Category = "Maverick|Quest")
	int32 GetQuestObjectiveValue(FName QuestId, FName ObjectiveId, int32 DefaultValue = 0) const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Quest")
	bool CompleteQuest(FName QuestId, bool bSaveImmediately = false);

	UFUNCTION(BlueprintPure, Category = "Maverick|Quest")
	bool IsQuestCompleted(FName QuestId) const;

private:
	UMVWorldStateSubsystem* GetWorldState() const;
	bool SaveDefaultIfRequested(bool bSaveImmediately) const;
};
