#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tables/MVActionTableTypes.h"
#include "Tables/MVCharacterTableTypes.h"
#include "MVActionComponent.generated.h"

class UAnimInstance;
class UAnimMontage;
class UMVStatComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionStatRecoveryPauseChanged, bool, bPaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionCostConsumed, int32, ActionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionPreparing, int32, ActionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionStarted, int32, ActionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnActionEnded, int32, ActionId, bool, bInterrupted);
DECLARE_DELEGATE_RetVal_OneParam(FVector, FMVResolveBufferedActionMovementInput, int32);
DECLARE_DELEGATE_RetVal_ThreeParams(bool, FMVCanConsumeBufferedAction, int32, const FVector&, bool);

/**
 * 공용 Action 실행 컴포넌트.
 *
 * 입력, 애니메이션 Notify, AI 판단처럼 서로 다른 액션 시작 요청을 ActionId 하나로 정규화해
 * 실행 가능 여부를 검사하고, chooser로 몽타주를 선택한 뒤 액션 런타임 상태를 관리한다.
 *
 * 책임:
 *   - CharacterIndex/ActionIndex/ActionStat 테이블에서 실행에 필요한 데이터를 찾는다.
 *   - 현재 액션, 비용 소비, 스탯 회복 정지, 몽타주 종료 콜백, 입력/이동/탈출 window 상태를 관리한다.
 *   - 버퍼된 액션의 추가 실행 문맥은 delegate를 통해 도메인 컴포넌트가 제공하도록 한다.
 *   - 이동 입력 차단은 액션 시작과 recovery escape 소비를 막고, recovery escape window는 후딜 입력과 소비를 허용한다.
 *
 * 라이프사이클:
 *   1) TryStartAction -> 실행 중이면 input buffer/recovery escape window 안에서 ActionId와 입력 문맥을 저장한다.
 *   2) TryStartActionById -> 비용을 검사하고 OnActionPreparing 후 chooser 몽타주를 재생한다.
 *   3) CompleteActiveAction/CancelActiveAction -> 자연 완료와 피동 취소를 명시적으로 처리한다.
 *   4) RecoveryEscapeWindow -> buffered action으로 현재 액션을 끊는 능동 탈출만 허용한다.
 *   5) Montage end/interruption -> notify state와 runtime state를 정리하고 종료 이벤트를 알린다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVActionComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Character")
	void SetCharacterIndexId(int32 NewCharacterIndexId);

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Character")
	int32 GetCharacterIndexId() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Character")
	int32 GetActionProfileId() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool TryStartAction(EMVActionId ActionId, float PlayRate = 1.0f, FName StartSection = NAME_None);

	bool TryStartActionById(int32 ActionId, float PlayRate = 1.0f, FName StartSection = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	void FinishActiveAction(bool bInterrupted = false);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	void CompleteActiveAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	void CancelActiveAction(float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	bool IsActionStatRecoveryPaused() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	bool IsActionRunning() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	int32 GetActiveActionId() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	UAnimMontage* GetActiveActionMontage() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Buffer")
	bool TryBufferAction(EMVActionId ActionId);

	bool TryBufferActionById(int32 ActionId);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Buffer")
	void ClearBufferedAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Buffer")
	void UpdateBufferedActionMovementInput(const FVector& MovementInputDirection);

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	bool HasBufferedAction() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	int32 GetBufferedActionId() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	bool HasBufferedActionMovementInput() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	FVector GetBufferedActionMovementInputDirection() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	bool IsConsumingBufferedAction() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	int32 GetConsumingBufferedActionId() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	bool HasConsumingBufferedActionMovementInput() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	FVector GetConsumingBufferedActionMovementInputDirection() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	bool IsInputBufferOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Buffer")
	bool TryStartBufferedAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void BeginInputBufferWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void EndInputBufferWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void BeginMovementInputBlock();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void EndMovementInputBlock();

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|NotifyState")
	bool IsMovementInputBlocked() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void BeginRecoveryEscapeWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void EndRecoveryEscapeWindow();

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|NotifyState")
	bool IsRecoveryEscapeWindowOpen() const;

	const FMVActionIndexRow* FindActionIndexRow(
		int32 ActionId,
		EMVActionType ExpectedActionType = EMVActionType::None) const;
	const FMVActionStatRow* FindActionStatRow(
		int32 ActionId,
		EMVActionType ExpectedActionType = EMVActionType::None) const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionStatRecoveryPauseChanged OnActionStatRecoveryPauseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionCostConsumed OnActionCostConsumed;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionPreparing OnActionPreparing;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionStarted OnActionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionEnded OnActionEnded;

	FMVResolveBufferedActionMovementInput ResolveBufferedActionMovementInput;
	FMVCanConsumeBufferedAction CanConsumeBufferedAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Table")
	FName CharacterIndexTableName = TEXT("CharacterIndex");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Table")
	FName ActionIndexTableName = TEXT("ActionIndex");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Table")
	FName ActionStatTableName = TEXT("ActionStat");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Character", meta = (ClampMin = "1"))
	int32 CharacterIndexId = MVCharacterIndexIds::Player;

private:
	void CacheOwnerReferences();
	const FMVCharacterIndexRow* FindCharacterIndexRow() const;
	int32 ResolveActionProfileId() const;
	static FString MakeActionIndexRowKey(int32 InActionProfileId, int32 InActionId);
	UAnimInstance* GetOwnerAnimInstance() const;
	UAnimMontage* ResolveActionMontage(int32 ActionId, const FMVActionIndexRow& ActionIndex) const;
	class UChooserTable* LoadActionChooserTable(int32 ActionId, const FMVActionIndexRow& ActionIndex) const;
	void HandleActionMontageEnded(UAnimMontage* Montage, bool bInterrupted, int32 ActionInstanceId);
	void BeginActionStatRecoveryPause();
	void EndActionStatRecoveryPause();
	void ResetActionNotifyState();
	void BufferAction(int32 ActionId, const FVector& MovementInputDirection);
	float GetActionStartStaminaCost(const FMVActionStatRow& ActionStat) const;
	float GetActionStartMPCost(const FMVActionStatRow& ActionStat) const;
	bool CanConsumeActionStartCost(const FMVActionStatRow& ActionStat) const;
	bool ConsumeActionStartCost(const FMVActionStatRow& ActionStat, int32 ActionId);

	UPROPERTY(Transient)
	TObjectPtr<UMVStatComponent> CachedStatComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveActionMontage;

	int32 ActiveActionId = INDEX_NONE;
	int32 ActiveActionInstanceId = INDEX_NONE;
	int32 NextActionInstanceId = 0;
	int32 ActionStatRecoveryPauseCount = 0;
	int32 InputBufferWindowCount = 0;
	int32 MovementInputBlockCount = 0;
	int32 RecoveryEscapeWindowCount = 0;
	int32 BufferedActionId = INDEX_NONE;
	FVector BufferedActionMovementInputDirection = FVector::ZeroVector;
	bool bBufferedActionHasMovementInput = false;
	bool bConsumingBufferedAction = false;
	int32 ConsumingBufferedActionId = INDEX_NONE;
	FVector ConsumingBufferedActionMovementInputDirection = FVector::ZeroVector;
	bool bConsumingBufferedActionHasMovementInput = false;
};
