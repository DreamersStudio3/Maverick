#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Tables/MVActionRowTableTypes.h"
#include "Tables/MVActionTableTypes.h"
#include "Tables/MVTableTypes.h"
#include "MVMovementActionTableTypes.generated.h"

/**
 * Dodge/Sprint처럼 이동 도메인에서 선택하고 소비하는 액션 데이터.
 *
 * ActionComponent는 이 row의 Montage/실행 플래그만 읽어 재생하고, 스태미나 비용은
 * 이동 도메인 컴포넌트가 row name으로 직접 읽어 처리한다. DataTable RowName이 실행 키다.
 * Dodge 이동 자체는 montage root motion이 담당한다.
 */
USTRUCT(BlueprintType)
struct MAVERICK_API FMVDodgeActionRow : public FMVActionRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Dodge|Stamina", meta = (ClampMin = "0.0"))
	float StaminaCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Dodge|Stamina")
	EMVActionResourceCostType StaminaCostType = EMVActionResourceCostType::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Dodge|Stamina", meta = (ClampMin = "0.0"))
	float MinRequiredStamina = 0.0f;
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVDodgeActionRowHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Table|Dodge")
	FDataTableRowHandle ActionRow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maverick|Table|Dodge")
	FName StartSection = NAME_None;

	bool IsValid() const
	{
		return ActionRow.DataTable && !ActionRow.RowName.IsNone();
	}

	void Reset()
	{
		ActionRow.DataTable = nullptr;
		ActionRow.RowName = NAME_None;
		StartSection = NAME_None;
	}
};

USTRUCT(BlueprintType)
struct MAVERICK_API FMVSprintActionRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Sprint|Stamina", meta = (ClampMin = "0.0"))
	float StaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Sprint|Stamina")
	EMVActionResourceCostType StaminaCostType = EMVActionResourceCostType::PerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Sprint|Stamina", meta = (ClampMin = "0.0"))
	float MinRequiredStamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Sprint|Stamina", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float SprintRestartStaminaPercent = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Sprint")
	bool bEnabled = true;
};
