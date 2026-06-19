#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tables/MVActionTableTypes.h"
#include "MVActionComponent.generated.h"

class UAnimInstance;
class UAnimMontage;
class UMVStatComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionStatRecoveryPauseChanged, bool, bPaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionCostConsumed, int32, ActionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionPreparing, int32, ActionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMVOnActionStarted, int32, ActionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMVOnActionEnded, int32, ActionId, bool, bInterrupted);

UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVActionComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	bool TryStartAction(EMVActionId ActionId, float PlayRate = 1.0f, FName StartSection = NAME_None);

	bool TryStartActionById(int32 ActionId, float PlayRate = 1.0f, FName StartSection = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action")
	void FinishActiveAction(bool bInterrupted = false);

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

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	bool HasBufferedAction() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	int32 GetBufferedActionId() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Action|Buffer")
	bool IsInputBufferOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void BeginInputBufferWindow();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|NotifyState")
	void EndInputBufferWindow();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Table")
	FName ActionIndexTableName = TEXT("ActionIndex");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Table")
	FName ActionStatTableName = TEXT("ActionStat");

private:
	void CacheOwnerReferences();
	UAnimInstance* GetOwnerAnimInstance() const;
	UAnimMontage* ResolveActionMontage(int32 ActionId, const FMVActionIndexRow& ActionIndex) const;
	class UChooserTable* LoadActionChooserTable(int32 ActionId, const FMVActionIndexRow& ActionIndex) const;
	void HandleActionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void BeginActionStatRecoveryPause();
	void EndActionStatRecoveryPause();
	void ResetActionNotifyState();
	float GetActionStartStaminaCost(const FMVActionStatRow& ActionStat) const;
	float GetActionStartMPCost(const FMVActionStatRow& ActionStat) const;
	bool CanConsumeActionStartCost(const FMVActionStatRow& ActionStat) const;
	bool ConsumeActionStartCost(const FMVActionStatRow& ActionStat, int32 ActionId);

	UPROPERTY(Transient)
	TObjectPtr<UMVStatComponent> CachedStatComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveActionMontage;

	int32 ActiveActionId = INDEX_NONE;
	int32 ActionStatRecoveryPauseCount = 0;
	int32 InputBufferWindowCount = 0;
	int32 BufferedActionId = INDEX_NONE;
};
