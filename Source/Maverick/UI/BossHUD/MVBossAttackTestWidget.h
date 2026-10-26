#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "MVBossAttackTestWidget.generated.h"

class AMVEnemy;
class AMVAIController;
class UButton;
class UStateTreeAIComponent;

/**
 * 기본 공격·Q·W·공명 폭주 시험 버튼과 기존 전투 실행 경로 연결
 * 생성 시 버튼 구독, 자동 AI 활성 중 게임 화면 마우스 고정·비활성 중 버튼 입력 제공
 * 클릭 시 보스 수동 제어 확보, 시험 중 커서 표시 유지
 * 보스 미지정 시 가장 가까운 Duelist 탐색, 공격·그로기·공명 실행 중 재입력 차단
 * 제거·보스 교체 시 시험 액션 정리와 기존 AI·대상·입력 복원; 공격 판정은 기존 Ability 책임
 */
UCLASS()
class MAVERICK_API UMVBossAttackTestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMVBossAttackTestWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Boss Attack Test")
	void SetTestBoss(AMVEnemy* InBoss);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Attack Test", meta = (ExposeOnSpawn = true))
	TObjectPtr<AMVEnemy> TestBoss;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Attack Test")
	TObjectPtr<UDataTable> AttackTable;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> BaseAttackButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> QAttackButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> WAttackButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> PazeChangeAttackButton;

private:
	UFUNCTION() void HandleBaseAttackClicked();
	UFUNCTION() void HandleQAttackClicked();
	UFUNCTION() void HandleWAttackClicked();
	UFUNCTION() void HandlePazeChangeAttackClicked();
	void ExecuteAttack(FName RowName);
	AMVEnemy* ResolveBoss();
	bool CanStartTest() const;
	void AcquireManualControl();
	void ReleaseManualControl();
	void RefreshButtons();
	void EnsureMouseInput();

	TWeakObjectPtr<AMVAIController> ControlledAI;
	TWeakObjectPtr<UStateTreeAIComponent> ControlledTree;
	TWeakObjectPtr<AActor> SavedAttackTarget;
	TWeakObjectPtr<AActor> SavedAITarget;
	bool bHasManualControl = false;
	bool bWasTreeRunning = false;
	bool bWasAITickEnabled = false;
	bool bWasMouseCursorVisible = false;
};
