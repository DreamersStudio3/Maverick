// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MVCombatComponent.h"

#include "Public/Tables/MVSkillDataTableColumn.h"

// Sets default values for this component's properties
UMVCombatComponent::UMVCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMVCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMVCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...
}

bool UMVCombatComponent::TryExecuteSkill(FName SkillName)
{
	// Todo: should concern about Input Window, Input Buffer 
	
	//UDataTable* SkillDataTable;

	//if (!SkillDataTable)
	{
	//	return false;
	}




	return false;
}

float UMVCombatComponent::GetRemainingCooldown(FName SkillName) const
{
	return 0.0f;
}

