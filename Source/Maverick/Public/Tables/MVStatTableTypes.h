#pragma once

#include "CoreMinimal.h"
#include "Tables/MVTableTypes.h"
#include "MVStatTableTypes.generated.h"

USTRUCT(BlueprintType)
struct MAVERICK_API FMVCharacterStatRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat")
	int32 StatId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|HP")
	float MaxHP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|HP")
	float CurrentHP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Stamina")
	float MaxStamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Stamina")
	float CurrentStamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Stamina")
	float StaminaRecoveryPerSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Stamina")
	float StaminaRecoveryDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|MP")
	float MaxMP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|MP")
	float CurrentMP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|MP")
	float MPRecoveryPerSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Attack")
	float AttackSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|MoveSpeed")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|MoveSpeed")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|MoveSpeed")
	float SprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Defence")
	float Defence = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Groggy")
	float MaxGroggy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Groggy")
	float CurrentGroggy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Groggy")
	float GroggyRecoveryPerSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Stat|Groggy")
	float GroggyRecoveryDelay = 0.0f;

	virtual void PostRead() override
	{
		RowId = StatId;
	}
};

USTRUCT(BlueprintType, meta = (MVTable = "PlayerStat"))
struct MAVERICK_API FMVPlayerStatRow : public FMVCharacterStatRow
{
	GENERATED_BODY()
};
