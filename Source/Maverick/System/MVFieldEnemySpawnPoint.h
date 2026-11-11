#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVFieldEnemySpawnPoint.generated.h"

class AMVEnemy;

/**
 * 일반 적 한 개체의 생성 위치와 초기 순찰 경로를 보관하는 필드 지점.
 * 최초 배치 적이 있으면 그 개체를 추적하고, 필드 초기화 시 클래스 기본 상태로 다시 생성한다.
 * 현재 적이 제거되어도 지점은 유지한다. 저장에서 소비한 일회성 지점과 보스는 재생성하지 않는다.
 */
UCLASS()
class MAVERICK_API AMVFieldEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AMVFieldEnemySpawnPoint();

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

private:
	static bool GatherResetPoints(UWorld& World, FName Field, TArray<AMVFieldEnemySpawnPoint*>& OutPoints);
	bool IsConsumed() const;
	bool SpawnEnemy();
	bool ReplaceEnemy();

	FTransform EnemyTransform = FTransform::Identity;
	TWeakObjectPtr<AActor> PatrolPath;
	TWeakObjectPtr<AMVEnemy> ActiveEnemy;
	bool bBoss = false;
	bool bOwnsSpawnedEnemy = false;
};
