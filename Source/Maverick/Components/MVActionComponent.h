#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Tables/MVActionRowTableTypes.h"
#include "MVActionComponent.generated.h"

class UAnimInstance;
class UAnimMontage;
class UMVStatComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnRecoverableStatRecoveryPauseChanged, bool, bPaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnActionPreparing, FName, ActionTableName, FName, ActionRowName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnActionStarted, FName, ActionTableName, FName, ActionRowName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMVOnActionEnded, FName, ActionTableName, FName, ActionRowName, bool, bInterrupted);

DECLARE_DELEGATE(FMVOnStatPauseStart);
DECLARE_DELEGATE(FMVOnStatPauseEnd);

/**
 * 공용 Action 실행 컴포넌트.
 *
 * 도메인 컴포넌트가 이미 선택한 상황별 액션 row를 몽타주 재생으로 실행하고 런타임 이벤트를 관리한다.
 *
 * 책임:
 *   - `ActionTableName + ActionRowName`으로 DataTable row를 찾아 몽타주를 재생한다.
 *   - 현재 액션, 스탯 회복 정지, 몽타주 종료 콜백, 입력/이동/탈출 window 상태를 관리한다.
 *   - 이동 입력 차단은 액션 시작을 막고, recovery window는 도메인 컴포넌트가 후딜 전환을 판단할 수 있게 알린다.
 *   - 이후 커스텀 몽타주 priority가 도입되면 더 높은 priority의 재생 요청이 낮은 priority 액션을 끊는
 *     정책도 이 컴포넌트의 현재 액션 전환 규칙에 포함된다.
 *
 * 라이프사이클:
 *   1) TryStartActionFromTable -> 외부 선택자가 넘긴 상황별 액션 테이블 row name으로 몽타주를 재생한다.
 *   2) CompleteActiveAction/CancelActiveAction -> 자연 완료와 피동 취소를 명시적으로 처리한다.
 *   3) Montage end/interruption -> notify state와 runtime state를 정리하고 종료 이벤트를 알린다.
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
	void SetCharacterIndexCode(FGameplayTag NewCharacterIndexCode);

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Character")
	FGameplayTag GetCharacterIndexCode() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool TryStartActionFromTable(FName ActionTableName, FName ActionRowName, FName StartSection = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool TryStartActionFromRowHandle(FDataTableRowHandle ActionRowHandle, FName StartSection = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool PauseActiveAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool ResumeActiveAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool TryTransitionActionFromTable(
		FName ActionTableName,
		FName ActionRowName,
		FName StartSection = NAME_None,
		float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool TryTransitionActionFromRowHandle(
		FDataTableRowHandle ActionRowHandle,
		FName StartSection = NAME_None,
		float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	void FinishActiveAction(bool bInterrupted = false);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	void CompleteActiveAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	void CancelActiveAction(float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	bool IsActionRunning() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	bool CanInterruptActiveAction() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	FName GetActiveActionTableName() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	FName GetActiveActionRowName() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action")
	UAnimMontage* GetActiveActionMontage() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Recovery")
	bool TryJumpActiveActionSection(FName SectionName);


	const FMVActionRow* FindActionRow(FName ActionTableName, FName ActionRowName) const;
	const FMVActionRow* FindActionRow(FDataTableRowHandle ActionRowHandle, FName& OutActionTableName, FName& OutActionRowName) const;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnRecoverableStatRecoveryPauseChanged OnRecoverableStatRecoveryPauseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionPreparing OnActionPreparing;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionStarted OnActionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Action|Event")
	FMVOnActionEnded OnActionEnded;

	// For StatComponent to pause stat recovery while an action is running.
	FMVOnStatPauseStart OnStatPauseStart;
	FMVOnStatPauseEnd OnStatPauseEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Character", meta = (Categories = "Character"))
	FGameplayTag CharacterIndexCode;

private:
	bool TryStartResolvedAction(
		FName ActionTableName,
		FName ActionRowName,
		const FMVActionRow& ActionRow,
		FName StartSection);
	UAnimInstance* GetOwnerAnimInstance() const;
	UAnimMontage* ResolveActionRowMontage(FName ActionTableName, FName ActionRowName, const FMVActionRow& ActionRow) const;
	void HandleActionMontageEnded(UAnimMontage* Montage, bool bInterrupted, int32 ActionInstanceId);
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveActionMontage;

	

	FName ActiveActionTableName = NAME_None;
	FName ActiveActionRowName = NAME_None;
	int32 ActiveActionInstanceId = INDEX_NONE;
	int32 NextActionInstanceId = 0;
	int32 RecoverableStatRecoveryPauseCount = 0;
	int32 InputBufferWindowCount = 0;
	int32 MovementInputBlockCount = 0;
	int32 RecoveryEscapeWindowCount = 0;
	bool bActiveActionCanBeInterrupted = true;
};
