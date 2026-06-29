// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Public/Tables/MVActionRowTableTypes.h"

#include "MVSkillDataTableColumn.generated.h"

/**
 * 
 */
class UMVAbilityBase;

USTRUCT(BlueprintType)
struct FMVSkillDataTableColumn : public FMVActionRow
{
	GENERATED_BODY()

public:
	FMVSkillDataTableColumn()
		: Damage(0.0f), CooldownDuration(0.0f), InterStageCooldown(0.0f), InputWindowDuration(0.0f),
		bIsChained(false), NextChainName(NAME_None)
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Data")
	TSubclassOf<UMVAbilityBase> AbilityReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Data")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Data")
	float CooldownDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Chaining")
	uint32 bIsChained : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Chaining")
	FName NextChainName;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Chaining")
	float InterStageCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Chaining")
	float InputWindowDuration;

};
