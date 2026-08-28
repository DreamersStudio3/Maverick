
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MVAIController.generated.h"

class UStateTreeAIComponent;

/**
 * AI 인지와 임시 BaseBoss 전투 시작 제어를 소유하는 공용 AIController.
 *
 * BeginPlay에서 `AttackTarget` 속성을 가진 BaseBoss Pawn의 StateTree를 정지해 전투 시작 전 Idle을
 * 유지한다. 임시 디버그 입력이 `BossPlayStart`를 활성화하면 StateTree를 시작하고, 플레이어 타겟을
 * Pawn의 Blueprint `AttackTarget` 속성에 전달한다.
 *
 * 라이프사이클:
 *   1) 생성자 -> AI Perception 구성과 갱신 delegate 연결
 *   2) BeginPlay -> BaseBoss 여부 판정 후 StateTree 대기 상태 적용
 *   3) 디버그 입력 -> 전투 시작 또는 공격 타겟 전달
 */
UCLASS()
class MAVERICK_API AMVAIController : public AAIController
{
	GENERATED_BODY()
	
public: 
	AMVAIController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	void SetBossPlayStartForDebug(bool bInBossPlayStart);
	void SetBossAttackTargetForDebug(AActor* InAttackTarget);
	bool IsBossDebugController() const;
	

protected:
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	

public:
	//인지한 타겟
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	AActor* TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Boss Debug")
	bool BossPlayStart = false;
	
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Boss Debug")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

private:
	void InitializeBossDebugGate();
	UStateTreeAIComponent* FindBossStateTreeComponent() const;
	bool WriteBossAttackTarget(AActor* InAttackTarget) const;
	bool bBossDebugGateInitialized = false;
};
