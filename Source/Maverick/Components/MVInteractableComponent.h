#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/MVInteractableInterface.h"
#include "Interaction/MVInteractionTypes.h"
#include "MVInteractableComponent.generated.h"

class UMVInteractableComponent;
class UMVDialogueWindow;
class UMVInteractionFlowDataAsset;
class UMVInteractionChoicePopup;
class UMVInteractionMenuWindow;
class UMVPopupBase;
class UMVWindowBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnInteractionRequested,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FMVOnInteractionMenuEntryRequested,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent,
	FGameplayTag, StepId,
	FMVMenuEntryData, EntryData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FMVOnInteractionCommandRequested,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent,
	FGameplayTag, StepId,
	FMVInteractionCommandRequest, CommandRequest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FMVOnInteractionCommandCompleted,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent,
	FGameplayTag, StepId,
	FMVInteractionCommandRequest, CommandRequest);

/**
 * 액터를 공통 상호작용 대상으로 노출하고 선택형 interaction flow를 실행하는 컴포넌트.
 *
 * detector는 이 컴포넌트의 prompt, priority, interact 가능 여부만 보고 후보를 고른다. 정의 기반 실행을
 * 켜면 flow asset 또는 inline instanced struct step 그래프를 따라 대화, 액션, 경고, 메뉴/선택지, window를
 * 순차 실행한다. command가 완료 대기를 요구하면 외부 애니메이션 notify나 도메인 로직이
 * `FinishInteractionCommand`를 호출할 때 다음 command나 step으로 넘어간다. 정의 기반 실행을 끄면 기존처럼
 * `OnInteractionRequested`만 방송한다.
 */
UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVInteractableComponent : public UActorComponent, public IMVInteractableInterface
{
	GENERATED_BODY()

public:
	UMVInteractableComponent();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnInteractionRequested OnInteractionRequested;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnInteractionMenuEntryRequested OnInteractionMenuEntryRequested;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnInteractionCommandRequested OnInteractionCommandRequested;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnInteractionCommandCompleted OnInteractionCommandCompleted;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void SetCanInteract(bool bInCanInteract);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void SetPromptText(FText InPromptText);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void SetUseInteractionDefinition(bool bInUseInteractionDefinition);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void SetInteractionFlowAsset(UMVInteractionFlowDataAsset* InInteractionFlowAsset);

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	UMVInteractionFlowDataAsset* GetInteractionFlowAsset() const { return InteractionFlowAsset; }

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void FinishInteractionCommand();

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	bool IsConfiguredInteractionRunning() const { return bConfiguredInteractionRunning; }

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	FGameplayTag GetActiveInteractionStepId() const { return ActiveStepId; }

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPromptText_Implementation(AActor* Interactor) const override;
	virtual int32 GetInteractionPriority_Implementation(AActor* Interactor) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction|Definition")
	bool bUseInteractionDefinition = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction|Definition", meta = (EditCondition = "bUseInteractionDefinition"))
	TObjectPtr<UMVInteractionFlowDataAsset> InteractionFlowAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction|Definition", meta = (EditCondition = "bUseInteractionDefinition", Categories = "Interaction.Flow.Step"))
	FGameplayTag InlineStartStepId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction|Definition", meta = (EditCondition = "bUseInteractionDefinition", BaseStruct = "/Script/Maverick.MVInteractionStepData", ExcludeBaseStruct))
	TArray<FInstancedStruct> InlineSteps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	bool bCanInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	int32 InteractionPriority = 0;

private:
	void ExecuteConfiguredInteraction(AActor* Interactor);
	void BeginConfiguredInteractionSession(AActor* Interactor);
	void EndConfiguredInteractionSession();
	bool ExecuteConfiguredStep(FGameplayTag StepId);
	void CompleteConfiguredStep(FGameplayTag NextStepId);
	bool ExecuteConfiguredCommands(const TArray<FInstancedStruct>& Commands, FGameplayTag NextStepId);
	bool ExecuteNextConfiguredCommand();
	bool TryHandleBuiltInGameplayEventCommand(const FMVInteractionCommandRequest& CommandRequest);
	FMVInteractionCommandRequest MakeCommandRequest(const FInstancedStruct& CommandInstance) const;
	FGameplayTag ResolveStartStepId() const;
	const TArray<FInstancedStruct>& ResolveInteractionSteps() const;
	FGameplayTag ResolveChoiceTransition(const FMVInteractionChoiceStepData& Step, FGameplayTag SelectedEntryId) const;
	FGameplayTag ResolveStepTransition(const FMVInteractionSelectionStepData& Step, FGameplayTag SelectedEntryId) const;
	const FInstancedStruct* FindInteractionStep(FGameplayTag StepId) const;
	class UMVUISubsystem* GetUISubsystem() const;

	UFUNCTION()
	void HandleConfiguredDialogueClosed(UMVDialogueWindow* ClosedDialogueWindow);

	UFUNCTION()
	void HandleConfiguredPopupClosed(UMVPopupBase* ClosedPopup);

	UFUNCTION()
	void HandleConfiguredMenuClosed(UMVInteractionMenuWindow* ClosedMenuWindow);

	UFUNCTION()
	void HandleConfiguredMenuEntrySelected(UObject* SourceObject, FMVMenuEntryData EntryData);

	UFUNCTION()
	void HandleConfiguredChoiceClosed(UMVPopupBase* ClosedChoicePopup);

	UFUNCTION()
	void HandleConfiguredChoiceEntrySelected(UObject* SourceObject, FMVMenuEntryData EntryData);

	UFUNCTION()
	void HandleConfiguredWindowDeactivated(UMVWindowBase* Window);

	UPROPERTY(Transient)
	TObjectPtr<AActor> ActiveInteractor;

	UPROPERTY(Transient)
	TObjectPtr<UMVDialogueWindow> ActiveConfiguredDialogueWindow;

	UPROPERTY(Transient)
	TObjectPtr<UMVPopupBase> ActiveConfiguredPopup;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionMenuWindow> ActiveConfiguredMenuWindow;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionChoicePopup> ActiveConfiguredChoicePopup;

	UPROPERTY(Transient)
	TObjectPtr<UMVWindowBase> ActiveConfiguredWindow;

	TArray<FInstancedStruct> ActiveCommandInstances;
	TArray<FInstancedStruct> PendingCommandsAfterMenuClose;
	FMVInteractionCommandRequest ActiveCommandRequest;
	FGameplayTag ActiveStepId;
	FGameplayTag PendingStepAfterMenuClose;
	FGameplayTag PendingStepAfterCommands;
	int32 ActiveCommandIndex = INDEX_NONE;
	bool bConfiguredInteractionRunning = false;
	bool bWaitingForConfiguredStep = false;
	bool bWaitingForConfiguredCommand = false;
	bool bBroadcastingConfiguredCommandRequest = false;
	bool bCompleteConfiguredCommandAfterRequest = false;
	bool bHasPendingStepAfterMenuClose = false;
	bool bHasPendingCommandsAfterMenuClose = false;
};
