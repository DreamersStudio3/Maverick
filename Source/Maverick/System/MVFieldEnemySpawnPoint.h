#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "MVFieldEnemySpawnPoint.generated.h"

class AMVEnemy;
class AMVItemPickup;
struct FMVDeathContext;
struct FMVEnemyDropTableRow;

/**
 * 배치 적의 사망 보상과 필드 초기화에 따른 적 재생성을 관리한다.
 * 한 생명당 한 번 보상 방식을 결정하고, 필드 초기화나 레벨 이탈 때 바닥 드롭을 정리한다.
 * 보상만 관찰하는 지점은 배치 적을 재생성하거나 제거하지 않는다.
 */
UCLASS()
class MAVERICK_API AMVFieldEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AMVFieldEnemySpawnPoint();

	FName GetFieldId() const { return FieldId; }
	FName GetSpawnId() const { return SpawnId; }

	static bool CanResetField(UWorld& World, FName Field);
	static FName ResolveDeathResetField(UWorld& World, FName PreferredField);
	static bool ResetField(UWorld& World, FName Field, const AActor* Player);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;

	UPROPERTY(EditInstanceOnly, Category = "Maverick|Field Reset")
	FName FieldId;

	UPROPERTY(EditInstanceOnly, Category = "Maverick|Field Reset")
	FName SpawnId;

	UPROPERTY(EditInstanceOnly, Category = "Maverick|Field Reset")
	TObjectPtr<AMVEnemy> InitialEnemy;

	UPROPERTY(EditAnywhere, Category = "Maverick|Field Reset")
	TSubclassOf<AMVEnemy> EnemyClass;

	UPROPERTY(EditInstanceOnly, Category = "Maverick|Enemy Drop", meta = (RowType = "/Script/Maverick.MVEnemyDropTableRow"))
	FDataTableRowHandle EnemyDropRow;

	UPROPERTY(EditInstanceOnly, Category = "Maverick|Enemy Drop")
	bool bObservePlacedEnemyOnly = false;

private:
	static bool GatherResetPoints(UWorld& World, FName Field, TArray<AMVFieldEnemySpawnPoint*>& OutPoints);
	bool IsObservationOnly() const { return bObservePlacedEnemyOnly || bBoss; }
	bool IsConsumed() const;
	bool SpawnEnemy();
	bool ReplaceEnemy();
	void ObserveEnemy(AMVEnemy& Enemy);
	void StopObservingEnemy();
	bool ClearEnemyDrops();
	void SpawnWorldDrops(UWorld& World, AMVEnemy& Enemy, const FMVEnemyDropTableRow& Row);

	UFUNCTION()
	void HandleEnemyDeath(const FMVDeathContext& DeathContext);

	FTransform EnemyTransform = FTransform::Identity;
	TWeakObjectPtr<AActor> PatrolPath;
	TWeakObjectPtr<AMVEnemy> ActiveEnemy;
	TArray<TWeakObjectPtr<AMVItemPickup>> EnemyDrops;
	bool bBoss = false;
	bool bOwnsSpawnedEnemy = false;
	bool bRewardHandledForLife = false;
};
