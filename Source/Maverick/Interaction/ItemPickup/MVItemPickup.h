#pragma once

#include "CoreMinimal.h"
#include "Enum/MVItemEnums.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interaction/MVInteractableInterface.h"
#include "MVItemPickup.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USphereComponent;
class UStaticMeshComponent;
class UMVWorldStateSubsystem;

/** 같은 구체에 입힐 등급별 색, 발광과 상승 효과의 크기. 길이 단위는 cm다. */
USTRUCT()
struct FMVItemPickupVisual
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Appearance")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (ClampMin = "1"))
	float BeamHeight = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (ClampMin = "1"))
	float BeamWidth = 16.0f;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (ClampMin = "1"))
	float HaloDiameter = 36.0f;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (ClampMin = "0"))
	float Brightness = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Appearance", meta = (ClampMin = "0", ClampMax = "16"))
	int32 ParticleCount = 0;
};

/**
 * 필드에 떨어진 아이템을 표시하고, 가까운 플레이어에게 한 묶음을 지급한다.
 * 배치 원점은 바닥에 둔다. 고정 배치는 필드와 배치 ID로 저장 슬롯의 소비 상태를
 * 따라간다. 지급 중 재호출을 막고 소비되면 감지용 구와 연출을 끈다.
 */
UCLASS(Blueprintable)
class MAVERICK_API AMVItemPickup : public AActor, public IMVInteractableInterface
{
	GENERATED_BODY()

public:
	AMVItemPickup();

	static AMVItemPickup* SpawnEnemyDrop(UWorld& World, TSubclassOf<AMVItemPickup> PickupClass, const FVector& GroundLocation,
		FGameplayTag ItemTag, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "Maverick|Item Pickup")
	EMVItemRarity GetResolvedRarity() const { return ResolvedRarity; }

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPromptText_Implementation(AActor* Interactor) const override;
	virtual int32 GetInteractionPriority_Implementation(AActor* Interactor) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UFUNCTION()
	void RestorePickupState();

	FText GetPickupIdentityError() const;
	void SetPickupVisible(bool bVisible);
	void RefreshVisuals();
	void UpdateParticles();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Item Pickup", meta = (AllowPrivateAccess = "true", Categories = "Item"))
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Item Pickup", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Maverick|Item Pickup")
	bool bSavePickup = true;

	UPROPERTY(EditInstanceOnly, Category = "Maverick|Item Pickup", meta = (EditCondition = "bSavePickup"))
	FName FieldId;

	UPROPERTY(EditInstanceOnly, Category = "Maverick|Item Pickup", meta = (EditCondition = "bSavePickup"))
	FName PickupId;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|Item Pickup", meta = (ClampMin = "1", Units = "cm"))
	float PickupDistance = 150.0f;

	UPROPERTY(VisibleAnywhere, Category = "Maverick|Item Pickup")
	TObjectPtr<USphereComponent> PickupBounds;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|Item Pickup|Appearance")
	TObjectPtr<UMaterialInterface> CoreMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|Item Pickup|Appearance")
	TObjectPtr<UMaterialInterface> GlowMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|Item Pickup|Appearance")
	FMVItemPickupVisual CommonVisual;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|Item Pickup|Appearance")
	FMVItemPickupVisual RareVisual;

	UPROPERTY(EditDefaultsOnly, Category = "Maverick|Item Pickup|Appearance")
	FMVItemPickupVisual EpicVisual;

	UPROPERTY(VisibleAnywhere, Category = "Maverick|Item Pickup")
	TObjectPtr<UStaticMeshComponent> CoreMesh;

	UPROPERTY(VisibleAnywhere, Category = "Maverick|Item Pickup")
	TArray<TObjectPtr<UStaticMeshComponent>> BeamMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Maverick|Item Pickup")
	TObjectPtr<UStaticMeshComponent> HaloMesh;

	UPROPERTY(VisibleAnywhere, Category = "Maverick|Item Pickup")
	TObjectPtr<UInstancedStaticMeshComponent> ParticleMeshes;

	EMVItemRarity ResolvedRarity = EMVItemRarity::UseItemType;
	TWeakObjectPtr<UMVWorldStateSubsystem> PickupSaveState;
	FDelegateHandle TableReloadHandle;
	TArray<FTransform> ParticleTransforms;
	float CurrentBeamHeight = 0.0f;
	float CurrentBeamWidth = 0.0f;
	float ParticleElapsed = 0.0f;
	double FeedbackUntil = 0.0;
	FText GrantFeedback;
	bool bAvailable = false;
	bool bValidPlacedPickup = true;
	bool bGrantInProgress = false;
	bool bCollected = false;
};
