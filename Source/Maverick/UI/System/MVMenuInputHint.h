#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

class ULocalPlayer;

namespace MVMenuInputHint
{
/** CommonUI의 현재 장치별 실제 키를 글자로 표시한다. 키가 없는 동작에는 키 안내를 붙이지 않는다. */
FText MakeActionLabel(const ULocalPlayer* LocalPlayer, const FDataTableRowHandle& Action, const FText& Label);
}
