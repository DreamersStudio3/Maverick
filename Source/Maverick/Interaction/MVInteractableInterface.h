#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MVInteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class MAVERICK_API UMVInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class MAVERICK_API IMVInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Interaction")
	bool CanInteract(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Interaction")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Interaction")
	FText GetInteractionPromptText(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Maverick|Interaction")
	int32 GetInteractionPriority(AActor* Interactor) const;
};
