#pragma once

#include "CoreMinimal.h"
#include "Tables/MVTableTypes.h"
#include "MVCharacterTableTypes.generated.h"

namespace MVCharacterIndexIds
{
	inline constexpr int32 Player = 1;
}

UENUM(BlueprintType)
enum class EMVCharacterKind : uint8
{
	None,
	Player,
	Enemy
};

/**
 * CharacterIndex 테이블의 캐릭터 데이터 정의 row.
 *
 * CharacterIndexId를 캐릭터 데이터의 공통 식별자로 사용하며, CharacterStat.StatId와 같은 값으로
 * 관리한다. ActionProfileId는 해당 캐릭터가 사용할 액션 목록과 애니메이션 프로필을 선택한다.
 */
USTRUCT(BlueprintType, meta = (MVTable = "CharacterIndex"))
struct MAVERICK_API FMVCharacterIndexRow : public FMVTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	int32 CharacterIndexId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	FName CharacterName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	EMVCharacterKind CharacterKind = EMVCharacterKind::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	int32 ActionProfileId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maverick|Table|Character")
	bool bEnabled = true;

	virtual void PostRead() override
	{
		RowId = CharacterIndexId;
	}
};
