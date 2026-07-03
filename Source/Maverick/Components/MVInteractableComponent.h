#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/MVInteractableInterface.h"
#include "Interaction/MVInteractionTypes.h"
#include "MVInteractableComponent.generated.h"

class UMVInteractableComponent;
class UMVDialogueWindow;
class UMVInteractionFlowDataAsset;
class UMVInteractionMenuWindow;
class UMVPopupBase;
class UMVWindowBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnInteractionRequested,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FMVOnInteractionMenuActionRequested,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent,
	FGameplayTag, StepId,
	FName, ActionName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FMVOnInteractionActionRequested,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent,
	FGameplayTag, StepId,
	FDataTableRowHandle, ActionRow,
	FName, StartSection);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FMVOnInteractionActionCompleted,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent,
	FGameplayTag, StepId,
	FDataTableRowHandle, ActionRow,
	FName, StartSection);

/**
 * 액터를 공통 상호작용 대상으로 노출하고 선택형 interaction flow를 실행하는 컴포넌트.
 *
 * detector는 이 컴포넌트의 prompt, priority, interact 가능 여부만 보고 후보를 고른다. 정의 기반 실행을
 * 켜면 flow asset 또는 inline instanced struct step 그래프를 따라 대화, 액션, 경고, 메뉴/선택지, window를
 * 순차 실행한다. 액션 step은 외부 애니메이션 notify나 도메인 로직이 `FinishInteractionAction`을 호출할
 * 때 다음 step으로 넘어간다. 정의 기반 실행을 끄면 기존처럼 `OnInteractionRequested`만 방송한다.
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
	FMVOnInteractionMenuActionRequested OnInteractionMenuActionRequested;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnInteractionActionRequested OnInteractionActionRequested;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnInteractionActionCompleted OnInteractionActionCompleted;

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
	void FinishInteractionAction();

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
	FGameplayTag ResolveStartStepId() const;
	const TArray<FInstancedStruct>& ResolveInteractionSteps() const;
	FGameplayTag ResolveStepTransition(const FMVInteractionSelectionStepData& Step, FName TriggerName) const;
	const FInstancedStruct* FindInteractionStep(FGameplayTag StepId) const;
	class UMVUISubsystem* GetUISubsystem() const;

	UFUNCTION()
	void HandleConfiguredDialogueClosed(UMVDialogueWindow* ClosedDialogueWindow);

	UFUNCTION()
	void HandleConfiguredPopupClosed(UMVPopupBase* ClosedPopup);

	UFUNCTION()
	void HandleConfiguredMenuClosed(UMVInteractionMenuWindow* ClosedMenuWindow);

	UFUNCTION()
	void HandleConfiguredMenuActionSelected(UObject* SourceObject, FName ActionName);

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
	TObjectPtr<UMVWindowBase> ActiveConfiguredWindow;

	FGameplayTag ActiveStepId;
	FGameplayTag PendingStepAfterMenuClose;
	bool bConfiguredInteractionRunning = false;
	bool bWaitingForConfiguredStep = false;
	bool bHasPendingStepAfterMenuClose = false;
};
