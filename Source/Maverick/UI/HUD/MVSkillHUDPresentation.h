#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"

class UTexture2D;
class UInputAction;
struct FEnhancedActionKeyMapping;

/** 선택 행에서 만드는 읽기 전용 화면 자료. 전투 상태나 저장 자료를 변경하지 않는다. */
struct FMVSkillHUDPresentation
{
	TArray<TSoftObjectPtr<UTexture2D>> Icons;
	FText Name;
	FText Status;
	bool bPrototype = false;
};

/** 그림과 키 안내의 공통 변환. 목록의 빈 그림도 유지해 연계 단계와 아이콘 인덱스를 일치시킨다. */
namespace MVSkillHUDPresentation
{
	MAVERICK_API FMVSkillHUDPresentation Build(const FDataTableRowHandle& Selected, TSoftObjectPtr<UTexture2D> Fallback);
	MAVERICK_API FText MakeKeyLabel(const TArray<FKey>& ActionKeys, const TArray<FKey>& ModifierKeys,
		bool bRequiresModifier, bool bGamepad, bool bTouch = false);
	MAVERICK_API FText MakeMappedKeyLabel(TConstArrayView<const FEnhancedActionKeyMapping> Mappings,
		const UInputAction* Action, const UInputAction* AdditionalModifier, bool bGamepad, bool bTouch = false);
}
