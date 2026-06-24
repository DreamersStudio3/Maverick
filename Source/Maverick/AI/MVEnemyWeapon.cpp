// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MVEnemyWeapon.h"

// Sets default values
AMVEnemyWeapon::AMVEnemyWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CombineWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CombineWeapon"));
	LeftWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftWeapon"));
	RightWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightWeapon"));
	
}

void AMVEnemyWeapon::AttachCombinedToHand(USkeletalMeshComponent* CharacterMesh)
{
	CombineWeapon->SetVisibility(true);
	LeftWeapon->SetVisibility(false);
	RightWeapon->SetVisibility(false);
	
	CombineWeapon->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
		,TEXT("BN_Weapon_R"));
}

void AMVEnemyWeapon::AttachDualToHands(USkeletalMeshComponent* CharacterMesh)
{
	CombineWeapon->SetVisibility(false);
	LeftWeapon->SetVisibility(true);
	RightWeapon->SetVisibility(true);
	
	LeftWeapon->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
		,TEXT("BN_Weapon_LSocket"));
	
	
	RightWeapon->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
		,TEXT("BN_Weapon_RSocket"));
	
}

// Called when the game starts or when spawned
void AMVEnemyWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMVEnemyWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

