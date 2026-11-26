#pragma once

#include "UI/Window/MVWeaponSkillSelectionWindow.h"
#include "MVWeaponPassiveSelectionWindow.generated.h"

/** 기존 자산과 열기 호출의 호환 타입. 실제 동작은 공통 스킬 선택 창이 담당한다. */
UCLASS(Abstract, Blueprintable)
class MAVERICK_API UMVWeaponPassiveSelectionWindow : public UMVWeaponSkillSelectionWindow
{
	GENERATED_BODY()
};
