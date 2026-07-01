// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MVCharacterBase.h"
#include "MVPlayerCharacter.generated.h"

UCLASS()
class MAVERICK_API AMVPlayerCharacter : public AMVCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMVPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
