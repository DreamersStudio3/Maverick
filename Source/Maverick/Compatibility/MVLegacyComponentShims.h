#pragma once

#include "CoreMinimal.h"
#include "Character/PC/InteractionDetector/MVPlayerInteractionDetector.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "InputCoreTypes.h"
#include "Tables/MVMovementActionTableTypes.h"
#include "UObject/SoftObjectPath.h"
#include "MVLegacyComponentShims.generated.h"

/**
 * Deprecated asset-load shim for the former player dodge component.
 *
 * The real dodge runtime now lives in AMVPlayerCharacter::Dodge. This class intentionally has no gameplay behavior;
 * it only preserves the old /Script/Maverick.MVDodgeComponent type so legacy Blueprint component templates can load
 * without being redirected into the UObject-backed player submodule.
 */
UCLASS(ClassGroup = (Maverick), HideDropdown, meta = (DisplayName = "MV Dodge Component (Legacy)"))
class MAVERICK_API UMVDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVDodgeComponent();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Dodge")
	void PrepareDodgeAction();

	UFUNCTION(BlueprintCallable, Category = "Maverick|Action|Dodge")
	void UpdateBufferedDodgeMovementInput(const FVector& MovementInputDirection);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Chooser")
	FSoftObjectPath DodgeChooserTable = TEXT("/Game/Table/Dodge/CHT_Dodge.CHT_Dodge");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Chooser")
	bool bUseNamingConventionWhenChooserUnavailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Table", meta = (ClampMin = "1"))
	int32 DefaultDodgeRowIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Action|Dodge|Recovery", meta = (ClampMin = "0.0", Units = "s"))
	float RecoveryDodgeTransitionBlendOutTime = 0.05f;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Maverick|Action|Dodge|Chooser")
	FMVDodgeActionRowHandle ChooserDodgeActionRowHandle;
};

/**
 * Deprecated asset-load shim for the former player interaction detector component.
 *
 * The real interaction runtime now lives in AMVPlayerCharacter::InteractionDetector. This class intentionally has no
 * gameplay behavior; it only preserves the old /Script/Maverick.MVInteractionDetectorComponent type for legacy assets.
 */
UCLASS(ClassGroup = (Maverick), HideDropdown, meta = (DisplayName = "MV Interaction Detector Component (Legacy)"))
class MAVERICK_API UMVInteractionDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMVInteractionDetectorComponent();

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
	UObject* GetFocusedInteractable() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	bool HasFocusedInteractable() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	int32 GetInteractableCandidateCount() const;

	UFUNCTION(BlueprintPure, Category = "Maverick|Interaction")
	int32 GetSelectedInteractableCandidateIndex() const;

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
};
