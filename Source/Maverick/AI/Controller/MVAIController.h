
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MVAIController.generated.h"

class UStateTreeAIComponent;

/**
 * AI 인지 대상·임시 BaseBoss 전투 시작 제어·보스 공격 순서 위치를 소유하는 공용 Controller
 * 생성 시 Perception 구성·갱신 연결, BeginPlay에서 AttackTarget 보유 Pawn의 StateTree 대기 적용
 * 등장 트리거의 대기·연출 중 전투와 디버그 입력 차단, 종료 후 대상·StateTree 활성화
 * 등장 제어 미사용 보스의 기존 K·L 디버그 경로 유지
 * 공격 순서 위치는 트리 재진입 중 보존, OnPossess에서 첫 후보로 초기화; 선택 정책은 공격 Task 책임
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

protected:
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AActor* TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Boss Debug")
	bool BossPlayStart = false;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Boss Intro")
	bool bBossIntroBlocked = false;

	UFUNCTION(BlueprintCallable, Category = "AI|Boss")
	void SetBossCombatEnabled(bool bEnabled, AActor* CombatTarget);

	void SetBossIntroBlocked(bool bBlocked);

	// 공격 트리 재진입 중 보존, 선택 순서와 범위 밖 예외 정책은 선택 Task 책임
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Boss Attack")
	int32 NextBossAttackChoiceIndex = 0;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Boss Debug")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

private:
	void InitializeBossDebugGate();
	void SetBossPlayStartForDebug(bool bInBossPlayStart);
	void SetBossAttackTargetForDebug(AActor* InAttackTarget);
	bool IsBossDebugController() const;
	UStateTreeAIComponent* FindBossStateTreeComponent() const;
	bool WriteBossAttackTarget(AActor* InAttackTarget) const;
	bool bBossDebugGateInitialized = false;
};
