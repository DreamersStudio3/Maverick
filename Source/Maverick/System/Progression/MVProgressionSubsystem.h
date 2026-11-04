#pragma once

#include "CoreMinimal.h"
#include "Struct/MVProgressionTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MVProgressionSubsystem.generated.h"

class UMVProgressionDefinition;
class UMVWorldStateSubsystem;

/**
 * 성장 정의와 저장된 플레이어 성장 상태를 결합하는 GameInstance 서브시스템.
 *
 * 데이터 애셋 로드와 공용 미리보기 계산을 담당하며 저장 데이터의 직접 소유권은 갖지 않는다.
 * 투자 확정 시 최신 저장 Revision을 재검사하고 UMVWorldStateSubsystem에 성장 레코드 전체 교체를 요청한다.
 * 라이프사이클:
 *   1) GameInstance 초기화 시 UMVWorldStateSubsystem 의존성 초기화
 *   2) UMVProgressionSettings의 성장 정의 동기 로드
 *   3) UI 요청 시 현재 저장 스냅샷을 이용한 미리보기 반환
 *   4) GameInstance 종료 시 로드된 정의 참조 해제
 */
UCLASS()
class MAVERICK_API UMVProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UMVProgressionSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Progression|Data")
	bool ReloadDefinition();

	UFUNCTION(BlueprintPure, Category = "Maverick|Progression|Data")
	bool IsDefinitionAvailable() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Progression")
	FMVProgressionEvaluation EvaluateAllocation(const TMap<FGameplayTag, int32>& PendingRanks) const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Progression")
	FMVProgressionEvaluation EvaluateCurrentProgression() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Progression")
	FMVProgressionEvaluation CommitAllocation(const TMap<FGameplayTag, int32>& PendingRanks, int32 ExpectedRevision);
	
	UFUNCTION(BlueprintCallable, Category = "Maverick|Progression|Currency")
	bool AddCurrency(int64 Amount);
	
	const UMVProgressionDefinition* GetDefinition() const { return LoadedDefinition; }

private:
	UMVWorldStateSubsystem* GetWorldState() const;

	UPROPERTY(Transient)
	TObjectPtr<UMVProgressionDefinition> LoadedDefinition;
};
