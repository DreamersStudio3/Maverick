#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Tables/MVTableTypes.h"
#include "MVCharacterTableTypes.generated.h"

/**
 * CharacterIndex 테이블의 캐릭터 데이터 정의 row.
 *
 * CharacterIndexCode를 캐릭터 데이터의 공통 row key로 사용하며, CharacterStat.CharacterIndexCode와 같은
 * GameplayTag로 관리한다. 액션 선택은 각 도메인 컴포넌트의 Chooser/DataTable 흐름에서 처리한다.
 */
USTRUCT(BlueprintType, meta = (MVTable = "CharacterIndex"))
struct MAVERICK_API FMVCharacterIndexRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character", meta = (Categories = "Character"))
	FGameplayTag CharacterIndexCode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	FName CharacterName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	bool bEnabled = true;

	virtual void PostRead() override
	{
		RowId = 0;
	}
};
