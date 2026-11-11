#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/MVInteractableInterface.h"
#include "MVRestPoint.generated.h"

class UBoxComponent;
class UMVRestPointWindow;
class USceneComponent;
class UStaticMeshComponent;

/**
 * 레벨에 배치하는 휴식 거점의 표시·감지 영역·기준 위치를 소유한다.
 * 기존 상호작용 요청을 월드 세션에 전달하고 제거될 때 해당 세션을 끝낸다.
 * Blueprint는 외형과 배치값을 설정한다.
 */
UCLASS(Blueprintable)
class MAVERICK_API AMVRestPoint : public AActor, public IMVInteractableInterface
{
	GENERATED_BODY()

public:
	AMVRestPoint();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPromptText_Implementation(AActor* Interactor) const override;
	virtual int32 GetInteractionPriority_Implementation(AActor* Interactor) const override;

	const FText& GetDisplayName() const { return DisplayName; }
	FName GetRestPointId() const { return RestPointId; }
	FName GetFieldId() const { return FieldId; }
	FTransform GetRespawnTransform() const;
	TSubclassOf<UMVRestPointWindow> GetMenuClass() const { return MenuClass; }
	bool IsInRange(const AActor& Player) const;
	bool HasLivingEnemyNearby() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rest Point")
	TObjectPtr<UStaticMeshComponent> RestPointMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rest Point")
	TObjectPtr<UBoxComponent> InteractionBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rest Point")
	TObjectPtr<UBoxComponent> SafetyBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rest Point")
	TObjectPtr<USceneComponent> RestAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rest Point")
	TObjectPtr<USceneComponent> RespawnAnchor;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Rest Point")
	FName RestPointId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rest Point")
	FName FieldId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rest Point")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Rest Point")
	TSubclassOf<UMVRestPointWindow> MenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rest Point", meta = (ClampMin = "0.0"))
	float InteractionDistance = 250.0f;
};
