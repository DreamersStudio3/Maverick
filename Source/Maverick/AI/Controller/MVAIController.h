
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MVAIController.generated.h"



UCLASS()
class MAVERICK_API AMVAIController : public AAIController
{
	GENERATED_BODY()
	
public: 
	AMVAIController();

	virtual void BeginPlay() override;
	

protected:
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	

public:
	//인지한 타겟
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	AActor* TargetActor;
	
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIPerception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;
	
	
	
};
