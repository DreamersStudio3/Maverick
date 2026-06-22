#pragma once

#include "CoreMinimal.h"
#include "Tables/MVTableTypes.h"
#include "MVStatTableTypes.generated.h"

/**
 * CharacterIndex row와 1:1로 연결되는 캐릭터 기본 스탯 row.
 *
 * StatId는 CharacterIndex.CharacterIndexId와 같은 값으로 관리하며, 런타임에서는 이 값을
 * row key로 사용해 캐릭터별 HP, 스태미너, MP, 이동/전투 기본 수치를 초기화한다.
 */
USTRUCT(BlueprintType, meta = (MVTable = "CharacterStat"))
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
