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
 * 생성 시 버튼 구독, 게임 배경 클릭으로 커서 숨김·화면 고정, J 키로 UI 조작 복귀
 * 클릭 시 StateTree 수동 제어 확보, Controller Tick 유지로 K·L 전투 복귀 허용
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

	UFUNCTION(BlueprintCallable, Category = "Boss Attack Test|Input")
	void SetUIMouseMode(bool bEnable);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss Attack Test|Input")
	bool bUIMouseMode = true;

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
	friend class FMVBossIntroTest;
	UFUNCTION() void HandleBaseAttackClicked();
	UFUNCTION() void HandleQAttackClicked();
	UFUNCTION() void HandleWAttackClicked();
	UFUNCTION() void HandlePazeChangeAttackClicked();
	void ExecuteAttack(FName RowName);
	AMVEnemy* ResolveBoss();
	bool CanStartTest() const;
	void AcquireManualControl();
	void ReleaseManualControl(bool bRestoreAI = true);
	void RefreshButtons();
	void EnsureMouseInput();
	bool CanControlMouseInput() const;

	TWeakObjectPtr<AMVAIController> ControlledAI;
	TWeakObjectPtr<UStateTreeAIComponent> ControlledTree;
	TWeakObjectPtr<AActor> SavedAttackTarget;
	TWeakObjectPtr<AActor> SavedAITarget;
	bool bHasManualControl = false;
	bool bWasTreeRunning = false;
	bool bWasMouseCursorVisible = false;
};
