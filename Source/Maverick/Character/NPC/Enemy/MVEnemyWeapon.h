// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MVEnemyWeapon.generated.h"

class USkeletalMeshComponent;

UCLASS()
class MAVERICK_API AMVEnemyWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMVEnemyWeapon();
	
	void AttachCombinedToHand(USkeletalMeshComponent* CharacterMesh);
	void AttachDualToHands(USkeletalMeshComponent* CharacterMesh);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
private:	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> CombineWeapon;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> LeftWeapon;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> RightWeapon;
	

};
