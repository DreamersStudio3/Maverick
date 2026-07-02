#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/MVInteractionTypes.h"
#include "System/MVWorldStateTypes.h"
#include "MVCheckpointActor.generated.h"

class AMVCheckpointActor;
class UMVInteractableComponent;
class UMVInteractionMenuWindow;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnCheckpointActivationEvent,
	AActor*, Interactor,
	AMVCheckpointActor*, Checkpoint);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnCheckpointMenuAction,
	AMVCheckpointActor*, Checkpoint,
	FName, ActionName);

/**
 * 상호작용으로 활성화하고 이후 메뉴를 여는 체크포인트 액터.
 *
 * 월드에 배치되면 기본적으로 비활성 상태이며, 첫 상호작용은 활성화 이벤트만 실행한다.
 * 활성화 이벤트 동안에는 공통 상호작용 세션을 열어 prompt 재표시를 막고, BP가 애니메이션 notify나
 * 완료 콜백에서 `FinishActivation`을 호출해야 활성화가 확정된다. 이후 상호작용은 CommonUI 메뉴를 열고,
 * 휴식/이동/비활성 기능 항목을 액션 이름으로 분기한다.
 */
UCLASS(Blueprintable)
class MAVERICK_API AMVCheckpointActor : public AActor
{
	GENERATED_BODY()

public:
	AMVCheckpointActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Checkpoint")
	void StartActivation(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Checkpoint")
	void FinishActivation();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Checkpoint")
	void OpenCheckpointMenu(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Checkpoint")
	void RestAtCheckpoint(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Checkpoint")
	void RefreshInteractableState();

	UFUNCTION(BlueprintPure, Category = "Maverick|Checkpoint")
	bool IsActivated() const { return bActivated; }

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Checkpoint|Event")
	FMVOnCheckpointActivationEvent OnCheckpointActivationStarted;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Checkpoint|Event")
	FMVOnCheckpointActivationEvent OnCheckpointActivated;

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Checkpoint|Event")
	FMVOnCheckpointMenuAction OnCheckpointMenuActionSelected;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Components")
	TObjectPtr<USphereComponent> InteractionVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Components")
	TObjectPtr<UMVInteractableComponent> InteractableComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Identity")
	FName CheckpointId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Identity")
	FName FieldId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|State")
	bool bActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Prompt")
	FText InactivePromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Prompt")
	FText ActivePromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Checkpoint|Menu")
	FMVInteractionMenuData CheckpointMenuData;

private:
	static FMVInteractionMenuData MakeDefaultCheckpointMenuData();
	FMVInteractionMenuData BuildCheckpointMenuData();
	void BeginCheckpointInteractionSession();
	void EndCheckpointInteractionSession();
	void SaveAsLastCheckpoint(bool bSaveImmediately);
	FName ResolveCheckpointId() const;
	FName ResolveCurrentMapName() const;
	class UMVUISubsystem* GetUISubsystem() const;

	UFUNCTION()
	void HandleInteractionRequested(AActor* Interactor, UMVInteractableComponent* Interactable);

	UFUNCTION()
	void HandleCheckpointMenuClosed(UMVInteractionMenuWindow* MenuWindow);

	UFUNCTION()
	void HandleCheckpointMenuActionSelected(UObject* SourceObject, FName ActionName);

	UPROPERTY(Transient)
	TObjectPtr<AActor> ActiveInteractor;

	UPROPERTY(Transient)
	TObjectPtr<UMVInteractionMenuWindow> ActiveMenuWindow;

	TMap<FName, FMVCheckpointSaveData> PendingTravelTargets;
	bool bActivating = false;
};
