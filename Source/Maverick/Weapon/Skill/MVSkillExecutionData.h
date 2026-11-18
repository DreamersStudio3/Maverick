#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

/** Ability 생성 전 전체 연계 자료를 검사. 실패 시 부분 연계를 반환하지 않는다. */
namespace MVSkillExecutionData
{
	MAVERICK_API bool ValidateDefinition(const FDataTableRowHandle& Start, TArray<FName>& OutRows, bool bAllowPrototype = true);
}
