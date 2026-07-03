#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "UObject/Object.h"
#include "MVPlayerInteractionDetector.generated.h"

class AActor;
class AMVPlayerCharacter;
class UPrimitiveComponent;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnFocusedInteractableChanged,
	UObject*, PreviousInteractable,
	UObject*, NewInteractable);

/**
 * PlayerCharacter 전용 상호작용 감지 서브모듈.
 *
 * 로컬 플레이어가 바라보는 범위 안의 interactable 후보를 모아 선택 상태와 UI 프롬프트를 갱신한다.
 * 플레이어 입력으로 상호작용을 실행하고, 대화/상호작용 세션이 열린 동안 같은 대상의 재감지와 해제를 관리한다.
 *
 * 책임:
 *   - PlayerCharacter의 위치/시야 기준으로 interactable 후보를 주기적으로 감지하고 우선순위 점수로 정렬한다.
 *   - 선택된 interactable 변경 이벤트와 InteractionPrompt 표시/숨김을 관리한다.
 *   - 대화창 스킵, 대화/메뉴 이탈 거리, 공통 상호작용 세션 억제 상태를 플레이어 상호작용 흐름 안에서 처리한다.
 *
 * 라이프사이클:
 *   1) PlayerCharacter BeginPlay -> Initialize로 owner를 저장하고 감지 타이머를 초기화한다.
 *   2) PlayerCharacter Tick -> Tick에서 입력 해제 게이트, 대화 이탈 상태, 후보 감지를 갱신한다.
 *   3) PlayerCharacter EndPlay -> Deinitialize로 포커스/프롬프트/억제 상태를 정리한다.
 */
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew)
class MAVERICK_API UMVPlayerInteractionDetector : public UObject
{
	GENERATED_BODY()

public:
	UMVPlayerInteractionDetector();

	virtual UWorld* GetWorld() const override;

	void Initialize(AMVPlayerCharacter& InOwnerCharacter);
	void Deinitialize();
	void Tick(float DeltaTime);

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnFocusedInteractableChanged OnFocusedInteractableChanged;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void SetInteractionDetectionEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void RefreshInteractable();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void ClearFocusedInteractable();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	bool SelectNextInteractable();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	bool SelectPreviousInteractable();

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	UObject* GetFocusedInteractable() const { return FocusedInteractable.Get(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	bool HasFocusedInteractable() const { return FocusedInteractable.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	int32 GetInteractableCandidateCount() const { return InteractionCandidates.Num(); }

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	int32 GetSelectedInteractableCandidateIndex() const { return SelectedCandidateIndex; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	bool bDetectionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction", meta = (ClampMin = "0.0"))
	float DetectionRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction", meta = (ClampMin = "0.0"))
	float DialogueEscapeRange = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction", meta = (ClampMin = "0.01"))
	float DetectionInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float DetectionHalfAngle = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	bool bUseViewCone = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	bool bRequireLineOfSight = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	TEnumAsByte<ECollisionChannel> LineOfSightChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	TArray<TEnumAsByte<ECollisionChannel>> InteractionObjectChannels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	FKey InteractionInputKey = EKeys::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	FText DefaultPromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Interaction|Debug")
	bool bDrawDebugDetection = false;

private:
	struct FMVInteractionCandidate
	{
		TWeakObjectPtr<UObject> InteractableObject;
		TWeakObjectPtr<AActor> InteractableActor;
		TWeakObjectPtr<UPrimitiveComponent> InteractableComponent;
		FVector Location = FVector::ZeroVector;
		float Score = 0.0f;
	};

	AMVPlayerCharacter* GetPlayerCharacter() const;
	AActor* GetOwnerActor() const;
	bool ShouldRunDetection() const;
	bool TryBuildCandidate(UPrimitiveComponent* OverlapComponent, const FVector& Origin, const FVector& ViewDirection, FMVInteractionCandidate& OutCandidate) const;
	bool HasLineOfSight(const FMVInteractionCandidate& Candidate) const;
	UObject* FindInteractableObject(UPrimitiveComponent* OverlapComponent) const;
	AActor* ResolveInteractableActor(UObject* InteractableObject, UPrimitiveComponent* FallbackComponent = nullptr) const;
	bool IsInteractableAvailable(UObject* InteractableObject) const;
	bool IsInteractableSuppressed(UObject* InteractableObject) const;
	bool IsInteractableWithinDetectionRange(UObject* InteractableObject) const;
	bool IsInteractableWithinDialogueEscapeRange(UObject* InteractableObject) const;
	bool IsDialoguePopupActive() const;
	bool IsDialogueInteractionBlocked() const;
	bool IsInteractionSessionActive() const;
	bool IsOwnerDead() const;
	bool SkipActiveDialoguePopup() const;
	void HideInteractionPrompt() const;
	void HideActiveDialoguePopup() const;
	void HideInteractionMenu() const;
	void RestoreDialogueCameraZoom() const;
	void ReleaseSuppressedInteractable(bool bHideDialogue);
	void UpdateDialogueEscapeState();
	void LockInteractionUntilInputReleased();
	void UpdateInteractionInputReleaseGate();
	bool IsInteractionInputHeld() const;
	int32 FindCandidateIndex(UObject* InteractableObject) const;
	bool SelectInteractableByOffset(int32 Offset);
	bool SetSelectedCandidateIndex(int32 NewIndex);
	void SetFocusedInteractable(UObject* NewInteractable);
	void UpdateInteractionPrompt();

	TArray<FMVInteractionCandidate> InteractionCandidates;
	TWeakObjectPtr<UObject> FocusedInteractable;
	TWeakObjectPtr<UObject> SuppressedInteractable;
	TWeakObjectPtr<AMVPlayerCharacter> OwnerPlayerCharacter;
	int32 SelectedCandidateIndex = INDEX_NONE;
	float TimeUntilNextDetection = 0.0f;
	bool bInitialized = false;
	bool bWaitForInteractionInputRelease = false;
};
