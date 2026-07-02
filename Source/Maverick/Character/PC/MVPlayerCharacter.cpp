// Fill out your copyright notice in the Description page of Project Settings.


#include "MVPlayerCharacter.h"
#include "Components/MVHitReactionComponent.h"


// Sets default values
AMVPlayerCharacter::AMVPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMVPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMVPlayerCharacter::BindDamageHandlers()
{
	Super::BindDamageHandlers();
	
	if (HitReactionComponent)
	{
		OnDamaged.RemoveDynamic(HitReactionComponent, &UMVHitReactionComponent::HandleDamaged);
		OnDamaged.AddUniqueDynamic(HitReactionComponent, &UMVHitReactionComponent::HandleDamaged);
	}
}

// Called every frame
void AMVPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMVPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
