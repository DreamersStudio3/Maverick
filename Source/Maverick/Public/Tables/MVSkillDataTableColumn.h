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
		: DamageMultiplier(1.0f), GroggyDamageMultiplier(1.0f), CooldownDuration(0.0f), bIsChained(false), NextChainName(NAME_None), InterStageCooldown(0.0f), InputWindowDuration(0.0f)
		
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Common")
	TSubclassOf<UMVAbilityBase> AbilityReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Common", meta = (ClampMin = "0.0"))
	float DamageMultiplier;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Common")
	float StaminaCost;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Common")
	float MpCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Common", meta = (ClampMin = "0.0"))
	float GroggyDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Skill")
	float CooldownDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Chain")
	uint32 bIsChained : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Chain")
	FName NextChainName;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Chain|Skill")
	float InterStageCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Chain|Skill")
	float InputWindowDuration;

};
