// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MVFinisherComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAVERICK_API UMVFinisherComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMVFinisherComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Public API
public:
	UFUNCTION(BlueprintCallable, Category = "Public API")
	bool TryFinisherMove();

private:
	bool CanFinisherMove(AActor* OutHitActor) const;

	bool MakeSphereTrace(AActor* OutHitActor) const;
	bool JudgeDistanceAndDirection(const AActor* HitActor) const;
	bool CheckThisActorGroggy(const AActor* HitActor) const;
		
};
