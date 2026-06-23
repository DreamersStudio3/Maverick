// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MVSkillDataTableColumn.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMVSkillDataTableColumn : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMVSkillDataTableColumn()
		: SkillAnim(nullptr), Damage(0.0f), CooldownDuration(0.0f)
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* SkillAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownDuration;



};
