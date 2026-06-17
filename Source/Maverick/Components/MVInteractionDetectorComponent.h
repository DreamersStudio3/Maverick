#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "MVInteractionDetectorComponent.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnFocusedInteractableChanged,
	UObject*, PreviousInteractable,
	UObject*, NewInteractable);

UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVInteractionDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVInteractionDetectorComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	bool ShouldRunDetection() const;
	bool TryBuildCandidate(UPrimitiveComponent* OverlapComponent, const FVector& Origin, const FVector& ViewDirection, FMVInteractionCandidate& OutCandidate) const;
	bool HasLineOfSight(const FMVInteractionCandidate& Candidate) const;
	UObject* FindInteractableObject(UPrimitiveComponent* OverlapComponent) const;
	AActor* ResolveInteractableActor(UObject* InteractableObject, UPrimitiveComponent* FallbackComponent = nullptr) const;
	bool IsInteractableAvailable(UObject* InteractableObject) const;
	bool IsInteractableSuppressed(UObject* InteractableObject) const;
	bool IsInteractableWithinDialogueEscapeRange(UObject* InteractableObject) const;
	bool IsDialogueWindowActive() const;
	bool IsDialogueInteractionBlocked() const;
	bool SkipActiveDialogueWindow() const;
	void HideActiveDialogueWindow() const;
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
	int32 SelectedCandidateIndex = INDEX_NONE;
	float TimeUntilNextDetection = 0.0f;
	bool bWaitForInteractionInputRelease = false;
};
