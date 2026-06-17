#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/MVInteractableInterface.h"
#include "MVInteractableComponent.generated.h"

class UMVInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMVOnInteractionRequested,
	AActor*, Interactor,
	UMVInteractableComponent*, InteractableComponent);

UCLASS(ClassGroup = (Maverick), meta = (BlueprintSpawnableComponent))
class MAVERICK_API UMVInteractableComponent : public UActorComponent, public IMVInteractableInterface
{
	GENERATED_BODY()

public:
	UMVInteractableComponent();

	UPROPERTY(BlueprintAssignable, Category = "Maverick|Interaction")
	FMVOnInteractionRequested OnInteractionRequested;

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void SetCanInteract(bool bInCanInteract);

	UFUNCTION(BlueprintCallable, Category = "Maverick|Interaction")
	void SetPromptText(FText InPromptText);

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPromptText_Implementation(AActor* Interactor) const override;
	virtual int32 GetInteractionPriority_Implementation(AActor* Interactor) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	bool bCanInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Interaction")
	int32 InteractionPriority = 0;
};
